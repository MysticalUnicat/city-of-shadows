#ifndef __LIBRARY_CORE_DATA_STRUCTURE_VECTOR_H__
#define __LIBRARY_CORE_DATA_STRUCTURE_VECTOR_H__

#include "memory.h"
#include "sort.h"

#define Vector(T) struct { uint32_t capacity; uint32_t length; T * data; }

#define VECTOR_INIT { 0, 0, NULL }

#define Vector_clear(V) \
  do {                        \
    (V)->length = 0;          \
  } while(0)

#define Vector_init(V) \
  do {                       \
    Vector_clear(V);   \
    (V)->capacity = 0;       \
    (V)->data = NULL;        \
  } while(0)

#define Vector_free(V)                                                          \
  do {                                                                                    \
    if((V)->data != NULL) {                                                               \
      memory_free((V)->data, (V)->capacity * sizeof(*(V)->data), alignof(*(V)->data)); \
    }                                                                                     \
    Vector_init(V);                                                                 \
  } while(0)

#define Vector_pop(V) ((V)->data + (--(V)->length))

#define Vector_push(V) ((V)->data + ((V)->length++))

#define Vector_set_capacity(V, C)                                                                                                  \
  ( ((C) > (V)->capacity) ?                                                                                                                  \
    (                                                                                                                                        \
     ((V)->data = memory_realloc((V)->data, (V)->capacity * sizeof(*(V)->data), (C) * sizeof(*(V)->data), alignof(*(V)->data))) != NULL ? \
      ((V)->capacity = (C), true) :                                                                                                          \
      false                                                                                                                                  \
    ) : true                                                                                                                                 \
  )

#define Vector_space_for(V, C) Vector_set_capacity(V, (((V)->length + (C)) + (((V)->length + (C)) >> 1)))

#define Vector_qsort(V, F, U) sort_qsort((V)->data, (V)->length, sizeof(*(V)->data), F, U)

#define Vector_bsearch(V, K, F, U) sort_bsearch(K, (V)->data, (V)->length, sizeof(*(V)->data), F, U)

#define Vector_remove_at(V, I)                                                             \
  do {                                                                                           \
      uint32_t __i = (I);                                                                        \
      (V)->length--;                                                                             \
      if((V)->length > 0 && (V)->length != __i) {                                                \
        memmove((V)->data + __i, (V)->data + __i + 1, ((V)->length - __i) * sizeof(*(V)->data)); \
      }                                                                                          \
  } while(0)

#define Vector_swap_pop(V, I)                                         \
  do {                                                                      \
    uint32_t __i = (I);                                                     \
    (V)->length--;                                                          \
    if((V)->length != __i) {                                                \
      memcpy((V)->data + __i, (V)->data + (V)->length, sizeof(*(V)->data)); \
    }                                                                       \
  } while(0)

#endif
