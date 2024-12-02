#include "script_local.h"
#include "configuration.h"
#include "editor.h"
#include "vfs.h"
#include "log.h"
#include "format.h"
#include "write.h"

#include <c2mir/c2mir.c>

#define SOURCE_NAMESPACE script

/*
static void _compile_script_to(CStr script_path, CStr mir_path) {
}

static void _compile_script(CStr script_path) {
  MStr mir_path = path_replace_extension(script_path, ".mir");
  _compile_script_to(script_path, mir_path);
  string_free(mir_path);
}

static void _found_script(CStr script_path) {
  MStr mir_path = path_replace_extension(script_path, ".mir");

  uint64_t script_mtime = vfs_mtime(script_path);
  uint64_t mir_mtime = vfs_mtime(mir_path);

  if(mir_mtime < script_mtime) {
    _compile_script_to(script_path, mir_path);
  }

  string_free(mir_path);
}
*/

bool script_initialize(void) {
  //vfs_find("**/*.script", _found_script);
  return false;
}

void FileFormat_Script_Parser_init(void **, struct Script_AST_Allocator *);
void FileFormat_Script_Parser_add_buffer(void * _parser, const void * data, uint64_t size);
void * FileFormat_Script_Parser_finish(void * _parser);

static struct {
  bool init;
  MIR_context_t ctx;
} _mir = { false };

static void _mir_init(void) {
  if(!_mir.init) {
    _mir.ctx = MIR_init();
    c2mir_init(_mir.ctx);
    _mir.init = true;
  }
}

#if 0
struct c2mir_macro_command {
  int def_p;              /* #define or #undef */
  const char *name, *def; /* def is used only when def_p is true */
};
struct c2mir_options {
  FILE *message_file;
  int debug_p, verbose_p, ignore_warnings_p, no_prepro_p, prepro_only_p;
  int syntax_only_p, pedantic_p, asm_p, object_p;
  size_t module_num;
  FILE *prepro_output_file; /* non-null for prepro_only_p */
  const char *output_file_name;
  size_t macro_commands_num, include_dirs_num;
  struct c2mir_macro_command *macro_commands;
  const char **include_dirs;
};
#endif

static bool _load(CStr path) {
  struct Reader * file = vfs_read(path);
  if(file == NULL) {
    TRACE(SOURCE_NAMESPACE, "could not open %s", path);
    return false;
  }
  TRACE(SOURCE_NAMESPACE, "opened %s", path);

  if(string_endswith(path, ".c")) {
    _mir_init();

    struct c2m_ctx *c2m_ctx = *c2m_ctx_loc(_mir.ctx);

    if (setjmp(c2m_ctx->env)) {
      compile_finish(c2m_ctx);
      goto error_file_finish;
    }

    struct c2mir_options ops;
    ops.message_file = NULL;
    ops.debug_p = 0;
    ops.verbose_p = 0;
    ops.ignore_warnings_p = 0;
    ops.no_prepro_p = 0;
    ops.prepro_only_p = 0;
    ops.syntax_only_p = 0;
    ops.pedantic_p = 0;
    ops.asm_p = 0;
    ops.object_p = 0;
    ops.module_num = 0;
    ops.prepro_output_file = NULL;
    ops.output_file_name = NULL;
    ops.macro_commands_num = 0;
    ops.include_dirs_num = 0;
    ops.macro_commands = NULL;
    ops.include_dirs = NULL;
    compile_init(c2m_ctx, &ops, (int (*)(void *))read_getc, file);

    compile_finish(c2m_ctx);
  } else if(string_endswith(path, ".script")) {
    struct Script_AST_Allocator allocator;

    Script_AST_Allocator_init(&allocator);

    void * parser;
    FileFormat_Script_Parser_init(&parser, &allocator);

    char buffer[256];
    int length;
    while((length = read_buffer(file, buffer, 256)) != 0) {
      FileFormat_Script_Parser_add_buffer(parser, buffer, length);
    }
    struct Script_AST * document = FileFormat_Script_Parser_finish(parser);

    {
      void Script_AST_format(const struct Script_AST * ast, struct Writer * w);

      MStr rewrite_path = format_alloc("%s.rewrite", path);
      struct Writer * out = write_to_file(rewrite_path, true);
      Script_AST_format(document, out);
      write_finish(out);
      string_free(rewrite_path);
    }

    Script_AST_Allocator_free(&allocator);
  } else {
    TRACE(SOURCE_NAMESPACE, "unknown script format for %s", path);
  }

  read_finish(file);
  return true;

error_file_finish:
  read_finish(file);
  return false;
}

void script_load(const char * path) {
  _load(path);
}

