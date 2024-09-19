#ifndef __LIBRARY_CORE_FORMAT_H__
#define __LIBRARY_CORE_FORMAT_H__

#include "memory.h"
#include "string.h"

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

typedef void (*format_Emit)(const uint8_t *ptr, size_t length, void *ud);

void format_emit_to_nothing(const uint8_t *ptr, size_t length, void *ud);

void format_emit_to_string(const uint8_t *ptr, size_t length, void *ud);
struct format_emit_to_string_UserData {
  uint8_t *string;
  size_t string_size;
  size_t position;
};

int format_v(format_Emit emit, void *emit_user_data, unsigned int limit, const char *format, va_list ap);

// everything goes through the above function, have some nice to use uses of it
static inline int format(format_Emit emit, void *emit_user_data, unsigned int limit, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int result = format_v(emit, emit_user_data, limit, format, ap);
  va_end(ap);
  return result;
}

static inline int format_count_v(const char *format, va_list ap) {
  return format_v(format_emit_to_nothing, NULL, (unsigned int)-1, format, ap);
}

static inline int format_count(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int count = format_count_v(format, ap);
  va_end(ap);
  return count;
}

static inline int format_string_limit_v(char * string, size_t string_size, unsigned int limit, const char *format, va_list ap) {
  struct format_emit_to_string_UserData user_data = {
    .string = string,
    .string_size = string_size,
    .position = 0
  };
  int result = format_v(format_emit_to_string, &user_data, limit, format, ap);
  if(string_size > 0) {
    size_t position = user_data.position >= string_size ? string_size - 1 : user_data.position;
    if(string_size && position < limit) {
      string[position] = 0;
    }
  }
  return result;
}

static inline int format_string_v(char * string, size_t string_size, const char *format, va_list ap) {
  return format_string_limit_v(string, string_size, (unsigned int)-1, format, ap);
}

static inline int format_string(char * string, size_t string_size, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int result = format_string_v(string, string_size, format, ap);
  va_end(ap);
  return result;
}

static inline MStr format_alloc_v(const char * format, va_list ap) {
  va_list ap_copy_1, ap_copy_2;

  va_copy(ap_copy_1, ap);
  int length = format_count_v(format, ap_copy_1);
  va_end(ap_copy_1);

  MStr result = string_allocate(length + 1);

  va_copy(ap_copy_2, ap);
  format_string_v(result, length + 1, format, ap_copy_2);
  va_end(ap_copy_2);

  return result;
}

static inline MStr format_alloc(const char * format, ...) {
  va_list ap;
  va_start(ap, format);
  MStr result = format_alloc_v(format, ap);
  va_end(ap);
  return result;
}

#endif
