#pragma once

#include "string.h"
#include "read.h"

enum VFS_Namespace {
  VFS_Namespacee_FILES,
  VFS_Namespacee_RES,
};

void vfs_set_default_namespace(enum VFS_Namespace namespace);

struct Writer;
struct Reader;

struct Reader * vfs_read(CStr path);

