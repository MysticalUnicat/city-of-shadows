#ifndef __LIBRARY_CORE_WRITE_H__
#define __LIBRARY_CORE_WRITE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

struct Writer;

struct Writer * write_to_file(const char * path, bool create);
void write_uint32(struct Writer * w, uint32_t value);
void write_string(struct Writer * w, const char * string);
void write_format_v(struct Writer * w, const char * format, va_list ap);

static inline void write_format(struct Writer * w, const char * format, ...) {
  va_list ap;
  va_start(ap, format);
  write_format_v(w, format, ap);
  va_end(ap);
}

void write_string_size_prefixed(struct Writer * w, const char * string);
uint64_t write_hole(struct Writer * w, size_t size);
void write_fill(struct Writer * w, uint64_t hole, const void * data, size_t data_size);
void write_finish(struct Writer * w);

#endif
