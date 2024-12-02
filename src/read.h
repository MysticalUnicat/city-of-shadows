#ifndef __LIBRARY_CORE_READ_H__
#define __LIBRARY_CORE_READ_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

#include "string.h"

struct Reader;

struct Reader * read_from_file(const char * path);
size_t read_buffer(struct Reader * r, void * buffer, size_t size);
int read_getc(struct Reader * r);
uint32_t read_uint32(struct Reader * r);
MStr read_string(struct Reader * r);
MStr read_string_size_prefixed(struct Reader * r);
void read_finish(struct Reader * r);

#endif
