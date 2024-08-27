#include "configuration.h"
#include "log.h"
#include "memory.h"
#include "sort.h"
#include "path.h"
#include "write.h"
#include "editor.h"
#include "resource.h"
#include "vfs.h"

#define SOURCE_NAMESPACE core.configuration

static CONFIGURATION_STRING(SOURCE_NAMESPACE, project_path, ".")
static CONFIGURATION_BOOLEAN(SOURCE_NAMESPACE, scan_project_parents, false)
static CONFIGURATION_STRING(SOURCE_NAMESPACE, main_pack, "")

static size_t values_cap = 0;
static size_t values_len = 0;
static struct configuration_Value * * values = NULL;
static uint32_t * values_sorted_by_name = NULL;

static int values_sorted_by_name_find(const void * _a, const void * _b, void *ud) {
  const char *a = *(const char **)_a;
  uint32_t b = *(uint32_t *)_b;
  return string_compare(a, values[b]->name);
}

static int values_sorted_by_name_sort(const void * _a, const void * _b, void *ud) {
  uint32_t a = *(uint32_t *)_a;
  uint32_t b = *(uint32_t *)_b;
  return string_compare(values[a]->name, values[b]->name);
}

MStr configuration_Value_as_string(struct configuration_Value * value) {
  MStr result;
  switch(value->kind) {
  case configuration_Value_NIL:
    result = string_clone("NIL");
    break;
  case configuration_Value_BOOLEAN:
    result = string_clone(*value->boolean ? "true" : "false");
    break;
  case configuration_Value_INTEGER:
    result = format_alloc("%lli", *value->integer);
    break;
  case configuration_Value_REAL:
    result = format_alloc("%f", *value->real);
    break;
  case configuration_Value_STRING:
    result = string_clone(*value->string);
    break;
  }
  return result;
}

static void set_from_string(struct configuration_Value * value, const char * string) {
  if(value->flag_immutable) {
    return;
  }
  switch(value->kind) {
  case configuration_Value_NIL:
    break;
  case configuration_Value_BOOLEAN:
    *value->boolean = STRING_CONSTANT("true") == string_hash(string);
    break;
  case configuration_Value_INTEGER:
    break;
  case configuration_Value_REAL:
    break;
  case configuration_Value_STRING:
    string_free(*value->string);
    *value->string = string_clone(string);
    break;
  }
}

struct configuration_Value * configuration_find(const char * name) {
  uint32_t * found = sort_bsearch(&name, values_sorted_by_name, values_len, sizeof(*values_sorted_by_name), values_sorted_by_name_find, NULL);
  if(found != NULL) {
    return values[*found];
  }
  return NULL;
}

void configuration_register(struct configuration_Value * value) {
  struct configuration_Value * val = configuration_find(value->name);
  if(val != NULL) {
    WARNING(SOURCE_NAMESPACE, "value '%s' already registered, ignoring", value->name);
    return;
  }
  size_t sorted_cap = values_cap;
  memory_grow(
    (void **)&values,
    sizeof(*values),
    alignof(*values),
    values_len + 1,
    &values_cap
  );
  memory_grow(
    (void **)&values_sorted_by_name,
    sizeof(*values_sorted_by_name),
    alignof(*values_sorted_by_name),
    values_len + 1,
    &sorted_cap
  );
  values[values_len] = value;
  values_sorted_by_name[values_len] = values_len;
  sort_qsort(values_sorted_by_name, values_len + 1, sizeof(*values_sorted_by_name), values_sorted_by_name_sort, NULL);
  values_len++;
}

void FileFormat_Configuration_Parser_init(void **);
void FileFormat_Configuration_Parser_add_buffer(void * _parser, const void * data, uint64_t size);
void FileFormat_Configuration_Parser_finish(void * _parser);

static bool _load(CStr path) {
  struct Reader * file = vfs_read(path);
  if(file == NULL) {
    return false;
  }
  // TRACE(SOURCE_NAMESPACE, "opened %s", path);

  void * parser;
  FileFormat_Configuration_Parser_init(&parser);

  char buffer[256];
  int length;
  while((length = read_buffer(file, buffer, 256)) != 0) {
    FileFormat_Configuration_Parser_add_buffer(parser, buffer, length);
  }
  FileFormat_Configuration_Parser_finish(parser);

  read_finish(file);
}

void configuration_initialize(void) {
  bool pack_loaded = false;
  MStr pack_path = NULL;
  bool execute_main_scene = false;

  if(string_size(main_pack) > 0) {
    if(!resource_load_pack(main_pack, false)) {
      ERROR("failed to load main pack %s", main_pack);
    } else {
      pack_loaded = true;
      pack_path = string_clone(main_pack);
    }
  }

  if(pack_loaded) {
    vfs_set_default_namespace(VFS_Namespacee_RES);
  }

  // vfs is setup now

  _load("merrin.config");

  if(pack_loaded) {
    _load("res://override.config");
    string_free(pack_path);
  }

  if(execute_main_scene) {
    _load("files://override.config");
  }
}

void configuration_set(const char * key, const char * value) {
  // TRACE(SOURCE_NAMESPACE, "configuration_set(%s, \"%s\")", key, value);
  struct configuration_Value * val = configuration_find(key);
  if(val != NULL) {
    set_from_string(val, value);
  }
}

void configuration_save(void) {
  char path[256];
  path_join(path, sizeof(path), project_path, "merrin.config");

  struct Writer * file = write_to_file(path, true);

  write_string(file, "; merrin configuration\n");

  MStr current_section = string_clone("");

  for(size_t value_index = 0; value_index < values_len; value_index++) {
    uint32_t i = values_sorted_by_name[value_index];

    if(values[i] == NULL || values[i]->flag_no_store) {
      continue;
    }

    CStr name = values[i]->name;
    CStr dot = string_find_reverse(name, '.');

    if(dot != NULL) {
      uint32_t current_section_length = string_size(current_section);
      if(current_section_length == 0 || string_compare_length(name, current_section, current_section_length) != 0) {
        string_free(current_section);
        current_section = string_clone_length(name, dot - name);
        write_format(file, "\n[%s]\n", current_section);
      }
      write_format(file, "%s = ", dot + 1);
    } else {
      write_format(file, "[%s]\n", name);
      name = "value";
    }

    MStr value_as_string = configuration_Value_as_string(values[i]);
    if(values[i]->help != NULL) {
      write_string(file, "; ");
      write_string(file, values[i]->help);
      write_string(file, "\n");
    }
    write_string(file, value_as_string);
    write_string(file, "\n");
    string_free(value_as_string);
  }
  write_finish(file);

  string_free(current_section);
}

