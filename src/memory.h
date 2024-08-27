#ifndef __LIBRARY_CORE_MEMORY_H__
#define __LIBRARY_CORE_MEMORY_H__

#include <stddef.h>
#include <stdbool.h>

#include <stdalign.h>

void * memory_alloc(size_t new_size, size_t alignment);
void * memory_realloc(void * ptr, size_t old_size, size_t new_size, size_t alignment);
void   memory_free(void * ptr, size_t old_size, size_t alignment);

static inline void memory_grow(void **ptr, size_t element_size, size_t alignment, size_t length, size_t *capacity) {
  size_t old_capacity = *capacity;
  if(length > old_capacity) {
    *capacity = length + 1;
    *capacity += *capacity >> 1;
    *ptr = memory_realloc(*ptr, element_size * old_capacity, element_size * *capacity, alignment);
  }
}

#define memory_new(T) (T *)memory_alloc(sizeof(T), alignof(T))
#define memory_del(PTR) memory_free(PTR, sizeof(*PTR), alignof(*PTR))

bool memory_copy(void * dest, size_t dest_size, const void * src, size_t count);
bool memory_set(void * dest, size_t dest_size, int ch, size_t count);

void * memory_clone_length(const void * data, size_t data_size, size_t alignment);

static inline void memory_clear(void * ptr, size_t size) {
  memory_set(ptr, size, 0, size);
}

#endif
