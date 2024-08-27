#include "memory.h"

#include <stdlib.h>
#include <string.h>

void * memory_alloc(size_t new_size, size_t alignment) {
  return malloc(new_size);
}

void * memory_realloc(void * ptr, size_t old_size, size_t new_size, size_t alignment) {
  return realloc(ptr, new_size);
}

void memory_free(void * ptr, size_t old_size, size_t alignment) {
  free(ptr);
}

bool memory_copy(void * dest, size_t dest_size, const void * src, size_t count) {
#if defined(__STDC_LIB_EXT1__)
  return memcpy_s(dest, dest_size, src, count) == 0;
#else
  (void)dest_size;
  memcpy(dest, src, count);
  return true;
#endif
}

bool memory_set(void * dest, size_t dest_size, int ch, size_t count) {
#if defined(__STDC_LIB_EXT1__)
  return memset_s(dest, dest_size, ch, count) == 0;
#else
  (void)dest_size;
  memset(dest, ch, count);
  return true;
#endif
}

void * memory_clone_length(const void * data, size_t data_size, size_t alignment) {
  void * result = memory_alloc(data_size, alignment);
  memory_copy(result, data_size, data, data_size);
  return result;
}
