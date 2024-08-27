#ifndef __LIBRARY_CORE_SORT_H__
#define __LIBRARY_CORE_SORT_H__

#include <stdbool.h>
#include <stddef.h>

bool sort_binary_search(const void *key, const void *ptr, size_t count, size_t size,
                          int (*comp)(const void *, const void *, void *ud), void *ud, const void **found,
                          size_t *not_found);

void sort_qsort(void *ptr, size_t count, size_t size, int (*comp)(const void *, const void *, void *ud), void *ud);

static inline void *sort_bsearch(const void *key, const void *ptr, size_t count, size_t size,
                                   int (*comp)(const void *, const void *, void *ud), void *ud) {
  const void *found;
  size_t not_found;
  if(count == 0) {
    return NULL;
  }
  return sort_binary_search(key, ptr, count, size, comp, ud, &found, &not_found) ? (void *)found : NULL;
}

#endif
