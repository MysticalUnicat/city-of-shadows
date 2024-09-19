#include "script.h"
#include "configuration.h"
#include "editor.h"

// #include <mir.h>

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

