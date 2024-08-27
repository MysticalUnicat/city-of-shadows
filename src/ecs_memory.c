#include "ecs.h"

#include <string.h>

ecs_Result ecs_malloc(
   size_t               size
  , size_t               alignment
  , void *             * out_ptr
) {
  *out_ptr = memory_alloc(size, alignment);
  if(*out_ptr == NULL) {
    // TODO make effort to cleanup old archetype blocks
    return ECS_ERROR_OUT_OF_MEMORY;
  }
  memset(*out_ptr, 0, size);
  return ECS_SUCCESS;
}

ecs_Result ecs_realloc(
   void               * ptr
  , size_t               old_size
  , size_t               new_size
  , size_t               alignment
  , void *             * out_ptr
) {
  *out_ptr = memory_realloc(ptr, old_size, new_size, alignment);
  if(*out_ptr == NULL) {
    return ECS_ERROR_OUT_OF_MEMORY;
  }
  if(old_size < new_size) {
    memset(((unsigned char *)*out_ptr) + old_size, 0, new_size - old_size);
  }
  return ECS_SUCCESS;
}

void ecs_free(
   void               * ptr
  , size_t               size
  , size_t               alignment
) {
  if(ptr != NULL && size > 0) {
    memory_free(ptr, size, alignment);
  }
}

