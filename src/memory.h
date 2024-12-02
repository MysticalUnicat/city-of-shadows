#ifndef __LIBRARY_CORE_MEMORY_H__
#define __LIBRARY_CORE_MEMORY_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>
#include <stdint.h>

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

void * memory_clone_length(const void * data, size_t data_size, size_t alignment);

typedef enum memory_Format {
  memory_Format_Unknown,

  memory_Format_Uint8,
  memory_Format_Uint16,
  memory_Format_Uint32,
  memory_Format_Uint64,

  memory_Format_Sint8,
  memory_Format_Sint16,
  memory_Format_Sint32,
  memory_Format_Sint64,

  memory_Format_Unorm8,
  memory_Format_Unorm16,

  memory_Format_Snorm8,
  memory_Format_Snorm16,

  memory_Format_Uscaled8,
  memory_Format_Uscaled16,

  memory_Format_Sscaled8,
  memory_Format_Sscaled16,

  memory_Format_Urgb8,

  memory_Format_Float16,
  memory_Format_Float32,
  memory_Format_Float64,

  memory_Format_COUNT
} memory_Format;

uint32_t memory_Format_size(memory_Format format);

typedef struct memory_Buffer {
  void *pointer;
  uintptr_t size;
} memory_Buffer;

typedef struct memory_SubBuffer {
  void *pointer;
  uint32_t count;
  uint32_t stride;
  memory_Format type_format;
  uint32_t type_length;
} memory_SubBuffer;

static inline memory_SubBuffer memory_SubBuffer_from_Buffer(memory_Buffer buffer, size_t buffer_offset, size_t buffer_limit, uint32_t stride,
                                                                        memory_Format type_format, uint32_t type_length) {
  return (memory_SubBuffer){.pointer = (void *)((uint8_t *)buffer.pointer + buffer_offset),
                                  .count = (buffer.size - buffer_limit) / stride,
                                  .stride = stride,
                                  .type_format = type_format,
                                  .type_length = type_length};
}

void memory_SubBuffer_copy_from_SubBuffer(memory_SubBuffer *dst, const memory_SubBuffer *src, uint32_t count);

int memory_SubBuffer_write(memory_SubBuffer *dst, uint32_t index, uint32_t count, memory_Format format, uint32_t src_stride, const void *src);
int memory_SubBuffer_read(const memory_SubBuffer *src, uint32_t index, uint32_t count, memory_Format format, uint32_t dst_stride, void *dst);

static inline bool memory_set(void *dest, size_t dest_size, int ch, size_t count) {
#if defined(__STDC_LIB_EXT1__) && ALIAS_MEMORY_C11_CONSTRAINTS
  extern int memset_s(void *, size_t, int, size_t);
  return memset_s(dest, dest_size, ch, count) == 0;
#else
  extern void *memset(void *, int, size_t);
  (void)dest_size;
  memset(dest, ch, count);
  return true;
#endif
}

static inline bool memory_clear(void *dest, size_t dest_size) { return memory_set(dest, dest_size, 0, dest_size); }

static inline bool memory_copy(void *dest, size_t dest_size, const void *src, size_t count) {
#if defined(__STDC_LIB_EXT1__) && ALIAS_MEMORY_C11_CONSTRAINTS
  extern int memcpy_s(void *, size_t, const void *, size_t);
  return memcpy_s(dest, dest_size, src, count) == 0;
#else
  extern void *memcpy(void *, const void *, size_t);
  (void)dest_size;
  memcpy(dest, src, count);
  return true;
#endif
}

static inline bool memory_move(void *dest, size_t dest_size, const void *src, size_t count) {
#if defined(__STDC_LIB_EXT1__) && ALIAS_MEMORY_C11_CONSTRAINTS
  extern int memmove_s(void *, size_t, const void *, size_t);
  return memmove_s(dest, dest_size, src, count) == 0;
#else
  extern void *memmove(void *, const void *, size_t);
  (void)dest_size;
  memmove(dest, src, count);
  return true;
#endif
}

static inline void *memory_clone(const void *src, size_t size, size_t alignment) {
  void *ptr = memory_alloc(size, alignment);
  if(ptr != NULL) {
    memory_copy(ptr, size, src, size);
  }
  return ptr;
}

#endif
