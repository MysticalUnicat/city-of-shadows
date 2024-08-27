#ifndef __LIBRARY_CORE_PATH_H__
#define __LIBRARY_CORE_PATH_H__

#include "format.h"

static inline void path_join(char * string, size_t string_size, const char * a, const char * b) {
  format_string(string, string_size, "%s/%s", a, b);
}

#endif
