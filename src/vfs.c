#include "vfs.h"
#include "resource.h"

#define SOURCE_NAMESPACE core.vfs

enum VFS_Namespace _default_namespace = VFS_Namespacee_FILES;

void vfs_set_default_namespace(enum VFS_Namespace namespace) {
  _default_namespace = namespace;
}

struct Reader * vfs_read(CStr path) {
  enum VFS_Namespace namespace = _default_namespace;
  if(string_compare_length(path, "res://", 6) == 0) {
    namespace = VFS_Namespacee_RES;
    path += 6;
  } else if(string_compare_length(path, "files://", 8) == 0) {
    namespace = VFS_Namespacee_RES;
    path += 8;
  }
  switch(namespace) {
  case VFS_Namespacee_FILES:
    return read_from_file(path);
  case VFS_Namespacee_RES:
    return resource_read(path);
  }
}
