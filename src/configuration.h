#ifndef __LIBRARY_CORE_CONFIGURATION_H__
#define __LIBRARY_CORE_CONFIGURATION_H__

#include <stdint.h>
#include <stdbool.h>

#include "initialization.h"
#include "string.h"

struct configuration_Value {
  enum {
    configuration_Value_NIL,
    configuration_Value_BOOLEAN,
    configuration_Value_INTEGER,
    configuration_Value_REAL,
    configuration_Value_STRING,
  } kind;
  const char * source_file;
  uint64_t source_line;
  const char * name;
  uint64_t name_hash;
  const char * help;
  uint32_t flag_no_store        :  1;
  uint32_t flag_immutable       :  1;
  uint32_t flag_unused          : 30;
  union {
    bool * boolean;
    int64_t * integer;
    float * real;
    MStr * string;
  };
  union {
    bool boolean_default;
    int64_t integer_default;
    float real_default;
    char * string_default;
  };
};

void configuration_register(struct configuration_Value * value);

struct configuration_Value * configuration_find(const char * name);

void configuration_set(const char * key, const char * value);

MStr configuration_Value_as_string(struct configuration_Value * value);

#define CONFIGURATION_VALUE(TYPE, UPPER, LOWER, NAMESPACE, NAME, DEFAULT, ...)        \
  TYPE NAME;                                                                                 \
  INIT_SECTION_PRIORITY(0) static void Merrin_Configuration_ ## NAME ## _init(void) { \
    NAME = DEFAULT;                                                                          \
    static struct configuration_Value value = {                                       \
      .kind = configuration_Value_ ## UPPER,                                          \
      .source_file = __FILE__,                                                               \
      .source_line = __LINE__,                                                               \
      .name = #NAMESPACE "." #NAME,                                                          \
      .name_hash = STRING_CONSTANT(#NAMESPACE "." #NAME),                             \
      .LOWER = &NAME,                                                                        \
      ## __VA_ARGS__                                                                         \
    };                                                                                       \
    value.LOWER ## _default = DEFAULT;                                                       \
    configuration_register(&value);                                                   \
  }

#define CONFIGURATION_BOOLEAN(NAMESPACE, NAME, DEFAULT, ...) CONFIGURATION_VALUE(   bool, BOOLEAN, boolean, NAMESPACE, NAME, DEFAULT, ## __VA_ARGS__)
#define CONFIGURATION_INTEGER(NAMESPACE, NAME, DEFAULT, ...) CONFIGURATION_VALUE(int64_t, INTEGER, integer, NAMESPACE, NAME, DEFAULT, ## __VA_ARGS__)
#define CONFIGURATION_REAL(NAMESPACE, NAME, DEFAULT, ...)    CONFIGURATION_VALUE(  float,    REAL,    real, NAMESPACE, NAME, DEFAULT, ## __VA_ARGS__)
#define CONFIGURATION_STRING(NAMESPACE, NAME, DEFAULT, ...)  CONFIGURATION_VALUE(MStr,  STRING,  string, NAMESPACE, NAME, string_clone(DEFAULT), ## __VA_ARGS__)

#endif
