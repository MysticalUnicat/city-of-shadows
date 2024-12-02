#include "string.h"
#include "memory.h"

#include <string.h>

// x x x x 0
// x x x 0 1
// x x 0 0 2
// x 0 0 0 3
// 0 0 0 0 4



static inline uint32_t get_capacity(MStr m) {
  return *(((uint32_t *)m) - 1);
}

static inline uint32_t get_remaining(MStr m) {
  uint32_t capacity = get_capacity(m);
  uint8_t * tail = ((uint8_t *)m) + capacity - 5;
  if(capacity >= 5 && tail[0] == 0) {
    return *(uint32_t *)&tail[1];
  } else {
    return tail[4];
  }
}

static inline void set_remaining(MStr m, uint32_t value) {
  uint32_t capacity = get_capacity(m);
  uint8_t * tail = ((uint8_t *)m) + capacity - 5;
  if(capacity >= 5 && value > 3) {
    tail[0] = 0;
    *(uint32_t *)&tail[1] = value;
  } else {
    tail[4] = value;
  }
}

static inline uint32_t get_length(MStr m) {
  uint32_t capacity = get_capacity(m);
  uint32_t remaining = get_remaining(m);
  return capacity - remaining - 1;
}

static inline void set_length(MStr m, uint32_t length) {
  uint32_t capacity = get_capacity(m);
  if(length <= capacity) {
    m[length] = 0;
    set_remaining(m, capacity - (length + 1));
  }
}

int string_size_m(MStr m) {
  if(m == NULL) {
    return 0;
  }
  return get_length(m);
}

int string_size_c(CStr c) {
  if(c == NULL) {
    return 0;
  }
  return strlen(c);
}

int string_size_a(AStr c) {
  return c.length;
}

static AStr AStr_from_CStr(CStr c) {
  return (AStr){.length = strlen(c), .base = c};
}

static AStr AStr_from_MStr(MStr m) {
  return (AStr){.length = get_length(m), .base = m};
}

bool string_equals(CStr a, CStr b) {
  while(*a && *a == *b) {
    a++;
    b++;
  }
  return *a == *b;
}

int string_compare(CStr a, CStr b) {
  while(*a && *a == *b) {
    a++;
    b++;
  }
  return (int)*a - (int)*b;
}

int string_compare_length(CStr a, CStr b, size_t length) {
  while(*a && *a == *b && length) {
    a++;
    b++;
    length--;
  }
  return length == 0 ? 0 : (int)*a - (int)*b;
}

bool string_endswith_aa(AStr a, AStr n) {
  if(a.length < n.length) {
    return false;
  }
  return string_compare_length(a.base + (a.length - n.length), n.base, n.length) == 0;
}

bool string_endswith_a(AStr a, CStr n) {
  return string_endswith_aa(a, AStr_from_CStr(n));
}

bool string_endswith_m(MStr m, CStr n) {
  return string_endswith_aa(AStr_from_MStr(m), AStr_from_CStr(n));
}

bool string_endswith_c(CStr c, CStr n) {
  return string_endswith_aa(AStr_from_CStr(c), AStr_from_CStr(n));
}

CStr string_find(CStr s, int n) {
  while(*s && *s != n) {
    s++;
  }
  return *s == n ? s : NULL;
}

CStr string_find_reverse_m(MStr s, int n) {
  uint32_t length = get_length(s) - 1;
  while(length > 0 && s[length] != n) {
    length--;
  }
  return s[length] == n ? s + length : NULL;
}

AStr string_find_reverse_a(AStr s, int n) {
  uint32_t length = s.length - 1;
  while(length > 0 && s.base[length] != n) {
    length--;
  }
  return (AStr){.length = s.length - length, .base = s.base[length] == n ? s.base + length : NULL};
}

CStr string_find_reverse_c(CStr s, int n) {
  CStr result = NULL;
  while(*s) {
    if(*s == n) {
      result = s;
    }
    s++;
  }
  return result;
}

uint64_t string_hash(CStr s) {
  uint64_t hash = STRING_HASH_INIT;
  while(*s) {
    hash ^= *s++;
    hash *= STRING_HASH_MULTIPLY;
  }
  return hash;
}

MStr string_allocate(uint32_t capacity) {
  uint32_t * header = (uint32_t *)memory_alloc(capacity + sizeof(uint32_t), 1);
  *header = capacity;
  MStr m = (MStr)(header + 1);
  set_remaining(m, 0);
  return m;
}

static MStr _clone(const char * base, uint32_t length) {
  uint32_t capacity = length + 1;
  MStr result = string_allocate(capacity);
  memory_copy(result, capacity, base, length);
  return result;
}

MStr string_clone_m(MStr m) {
  if(m == NULL) {
    return NULL;
  }
  return _clone(m, get_length(m));
}

MStr string_clone_a(AStr m) {
  return _clone(m.base, m.length);
}

MStr string_clone_c(CStr s) {
  if(s == NULL) {
    return NULL;
  }
  return _clone(s, strlen(s));
}

MStr string_clone_length_m(MStr s, size_t length) {
  uint32_t s_length = get_length(s);
  if(length > s_length) {
    length = s_length;
  }
  MStr result = string_allocate(length + 1);
  memory_copy(result, length, s, length);
  return result;
}

MStr string_clone_length_c(CStr s, size_t length) {
  uint32_t s_length = string_size_c(s);
  if(length > s_length) {
    length = s_length;
  }
  MStr result = string_allocate(length + 1);
  memory_copy(result, length, s, length);
  return result;
}

MStr _grow(MStr previous, size_t target_length) {
  MStr result = previous;
  uint32_t capacity = get_capacity(previous);
  uint32_t length = get_length(previous);
  if(target_length > capacity) {
    uint32_t target_capacity = target_length + 1;
    target_capacity += target_capacity >> 1;
    result = string_allocate(target_capacity);
    set_length(result, length);
    memory_copy(result, target_capacity, previous, length);
    string_free(previous);
  }
  return result;
}

static MStr _concatenate_m(MStr left, const char * right, size_t right_size) {
  size_t left_size = string_size_m(left);
  uint32_t total_size = left_size + right_size;
  left = _grow(left, total_size);
  memory_copy(left + left_size, get_capacity(left) - left_size, right, right_size);
  set_length(left, total_size);
  return left;
}

static MStr _concatenate_c(const char * left, size_t left_size, const char * right, size_t right_size) {
  uint32_t total_size = left_size + right_size;
  MStr result = string_allocate(total_size);
  memory_copy(result + 0, total_size, left, left_size);
  memory_copy(result + left_size, total_size - left_size, right, right_size);
  return result;
}

MStr string_concatenate_mm(MStr left, MStr right) {
  return _concatenate_m(left, right, get_length(right));
}

MStr string_concatenate_ma(MStr left, AStr right) {
  return _concatenate_m(left, right.base, right.length);
}

MStr string_concatenate_mc(MStr left, CStr right) {
  return _concatenate_m(left, right, strlen(right));
}

MStr string_concatenate_cm(CStr left, MStr right) {
  return _concatenate_c(left, strlen(left), right, get_length(right));
}

MStr string_concatenate_cc(CStr left, CStr right) {
  return _concatenate_c(left, strlen(left), right, strlen(right));
}

static MStr _concatenate_length_m(MStr left, const char * right, size_t right_size, size_t length) {
  size_t left_size = string_size_m(left);
  if(left_size > length) {
    left_size = length;
  }
  if(right_size > length - left_size) {
    right_size = length - left_size;
  }
  uint32_t total_size = left_size + right_size;
  left = _grow(left, total_size);
  memory_copy(left + left_size, get_capacity(left) - left_size, right, right_size);
  set_length(left, total_size);
  return left;
}

static MStr _concatenate_length_c(const char * left, size_t left_size, const char * right, size_t right_size, size_t length) {
  if(left_size > length) {
    left_size = length;
  }
  if(right_size > length - left_size) {
    right_size = length - left_size;
  }
  uint32_t total_size = left_size + right_size;
  MStr result = string_allocate(total_size);
  memory_copy(result + 0, total_size, left, left_size);
  memory_copy(result + left_size, total_size - left_size, right, right_size);
  return result;
}

MStr string_concatenate_length_mm(MStr left, MStr right, size_t length) {
  return _concatenate_length_m(left, right, get_length(right), length);
}

MStr string_concatenate_length_mc(MStr left, CStr right, size_t length) {
  return _concatenate_length_m(left, right, strlen(right), length);
}

MStr string_concatenate_length_cm(CStr left, MStr right, size_t length) {
  return _concatenate_length_c(left, strlen(left), right, get_length(right), length);
}

MStr string_concatenate_length_cc(CStr left, CStr right, size_t length) {
  return _concatenate_length_c(left, strlen(left), right, strlen(right), length);
}

MStr string_append_m(MStr left, uint8_t byte) {
  return _concatenate_m(left, &byte, 1);
}

MStr string_append_c(CStr left, uint8_t byte) {
  return _concatenate_c(left, strlen(left), &byte, 1);
}

void string_clear(MStr s) {
  if(s == NULL) return;
  set_length(s, 0);
}

void string_free(MStr m) {
  if(m == NULL) {
    return;
  }
  uint32_t * header = (uint32_t *)m - 1;
  memory_free(header, *header + sizeof(uint32_t), 1);
}

