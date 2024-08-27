#include "resource.h"
#include "platform.h"
#include "endian.h"
#include "memory.h"
#include "vector.h"
#include "log.h"
#include "format.h"

#define SOURCE_NAMESPACE core.resource

struct pack_File {
  MStr path;
  uint32_t offset;
  uint32_t length;
  uint32_t source_file;
};

static Vector(struct platform_File *) _source_files;
static Vector(struct pack_File) _files;
static uint32_t * _files_sorted_by_path;

struct _load_key {
  CStr base;
  uint32_t length;
};

int _load_key_comp(const void * _a, const void * _b, void * ud) {
  struct _load_key * key = (struct _load_key *)_a;
  uint32_t index = *(uint32_t *)_b;
  return string_compare_length(key->base, _files.data[index].path, key->length);
}

int _sort_files_comp(const void * _a, const void * _b, void * ud) {
  uint32_t index_a = *(uint32_t *)_a;
  uint32_t index_b = *(uint32_t *)_b;
  return string_compare(_files.data[index_a].path, _files.data[index_b].path);
}

struct MPCK_Header {
  char prefix[4];
  uint32_t version;
  uint32_t flags;
  uint32_t num_pack_files;
};

bool resource_load_pack(CStr path, bool overwrite) {
  struct platform_File * source_file = memory_alloc(platform_File_size(), 4);
  if(!platform_File_open_synchronous(source_file, path, platform_File_READ)) {
    goto on_error;
  }
  uint64_t num_read;
  struct MPCK_Header header;
  if(!platform_File_read_synchronous(source_file, -1, &header, sizeof(header), &num_read)) {
    goto on_error_close;
  }
  if(header.prefix[0] != 'M' && header.prefix[1] != 'P' && header.prefix[2] != 'C' && header.prefix[3] != 'K') {
    ERROR(SOURCE_NAMESPACE, "invalid pack header");
    goto on_error_close;
  }
  endian_little(&header.version);
  endian_little(&header.flags);
  endian_little(&header.num_pack_files);
  // array(uint32_t, 2, num_pack_files)
  uint32_t * file_entries = memory_alloc(sizeof(*file_entries) * header.num_pack_files * 2, alignof(*file_entries));
  if(!platform_File_read_synchronous(source_file, -1, file_entries, sizeof(*file_entries) * header.num_pack_files * 2, &num_read)) {
    goto on_error_close;
  }
  // atop(fold(max), select(0))
  uint32_t max_path_size = 0;
  for(uint32_t i = 0; i < header.num_pack_files; i++) {
    max_path_size = file_entries[i * 2 + 0] > max_path_size ? file_entries[i * 2 + 0] : max_path_size;
  }
  // fold(add)
  uint64_t paths_size = 0;
  uint64_t files_size = 0;
  for(uint32_t i = 0; i < header.num_pack_files; i++) {
    paths_size += file_entries[i * 2 + 0];
    files_size += file_entries[i * 2 + 1];
  }
  struct _load_key key;
  key.base = string_allocate(paths_size);
  if(!platform_File_read_synchronous(source_file, -1, (void *)key.base, paths_size, &num_read)) {
    goto on_error_paths;
  }
  uint64_t file_offset = 0;
  Vector_space_for(&_source_files, 1);
  *Vector_push(&_source_files) = source_file;
  uint32_t old_files_length = _files.length;
  for(uint32_t i = 0; i < header.num_pack_files; i++) {
    uint32_t path_length = file_entries[i * 2 + 0];
    uint32_t file_length = file_entries[i * 2 + 1];
    key.length = path_length;
    uint32_t * found_file_index;
    found_file_index = (uint32_t *)sort_bsearch(&key, _files_sorted_by_path, _files.length, sizeof(*_files_sorted_by_path), _load_key_comp, NULL);
    if(found_file_index == NULL) {
      Vector_space_for(&_files, 1);
      *Vector_push(&_files) = (struct pack_File) {
        .path = format_alloc("%.*s", key.length, key.base),
        .offset = file_offset,
        .length = file_length,
        .source_file = _source_files.length - 1
      };
    } else if(overwrite) {
      struct pack_File * pack_file = &_files.data[*found_file_index];
      pack_file->offset = file_offset;
      pack_file->length = file_length;
      pack_file->source_file = _source_files.length - 1;
    }
    key.base += key.length;
    file_offset += file_length;
  }
  if(old_files_length != _files.length) {
    _files_sorted_by_path = memory_realloc(_files_sorted_by_path, sizeof(*_files_sorted_by_path) * old_files_length, sizeof(*_files_sorted_by_path) * _files.length, alignof(*_files_sorted_by_path));
    for(uint32_t i = old_files_length; i < _files.length; i++) {
      _files_sorted_by_path[i] = i;
    }
    sort_qsort(_files_sorted_by_path, _files.length, sizeof(*_files_sorted_by_path), _sort_files_comp, NULL);
  }
  string_free((MStr)key.base);
  return true;
on_error_paths:
  string_free((MStr)key.base);
on_error_close:
  platform_File_close_synchronous(source_file);
on_error:
  memory_free(source_file, platform_File_size(), 4);
  return false;
}

struct Reader * read_from_resource(struct platform_File * source_file, uint64_t start, uint64_t length);

struct Reader * resource_read(CStr path) {
  struct _load_key key = {.base = path, .length = string_size(path)};
  uint32_t * found_file_index = (uint32_t *)sort_bsearch(&key, _files_sorted_by_path, _files.length, sizeof(*_files_sorted_by_path), _load_key_comp, NULL);
  if(found_file_index == NULL) {
    return NULL;
  }
  struct pack_File * pack_file = &_files.data[*found_file_index];
  return read_from_resource(_source_files.data[pack_file->source_file], pack_file->offset, pack_file->length);
}

