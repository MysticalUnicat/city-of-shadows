#include "sort.h"

#include <stdint.h>

bool sort_binary_search(const void *key, const void *ptr, size_t count, size_t size,
                          int (*comp)(const void *, const void *, void *ud), void *ud, const void **found,
                          size_t *not_found) {
  size_t low = 0;
  size_t high = count;
  int64_t middle;
  while(high >= low && high <= count) {
    middle = (low + high) >> 1;
    const void *test = (uint8_t *)ptr + middle * size;
    int cmp = comp(key, test, ud);
    if(cmp == 0) {
      *found = test;
      return true;
    } else if(cmp > 0) {
      low = middle + 1;
    } else {
      high = middle - 1;
    }
  }
  *not_found = middle;
  return false;
}

static inline void quicksort_swap(uint8_t *a, uint8_t *b, size_t size) {
  while(size--) {
    uint8_t c = *a;
    *a = *b;
    *b = c;
    a++;
    b++;
  }
}

static void quicksort(uint8_t *low, uint8_t *high, size_t size, int (*comp)(const void *, const void *, void *ud),
                      void *ud) {
  if(low >= high) {
    return;
  }

  size_t count = ((high - low) / size) + 1;

#define LESS_THAN_EQUAL(I, J) (comp(I, J, ud) <= 0)
#define SWAP(I, J) quicksort_swap(I, J, size)
#define SORT2(I, J)                                                                                                    \
  do {                                                                                                                 \
    if(LESS_THAN_EQUAL(J, I)) {                                                                                        \
      SWAP(I, J);                                                                                                      \
    }                                                                                                                  \
  } while(false)

  SORT2(low, high);
  if(count == 2) {
    return;
  }

  // find mid
  uint8_t *mid = low + ((high - low) / (size << 1)) * size;

  // make sure low < mid
  SORT2(low, mid);

  // if count 3, make sure low < mid < high then return
  if(count == 3) {
    SORT2(mid, high);
    return;
  }

  // start partiion
  // place in this order: low < high < mid (using position high as pivot)
  SORT2(high, mid);
  uint8_t *pivot = low - size;

  for(uint8_t *scan = low; scan < high; scan += size) {
    if(LESS_THAN_EQUAL(scan, high)) {
      pivot += size;
      SWAP(pivot, scan);
    }
  }
  pivot += size;
  SWAP(pivot, high);
  // done partition

  quicksort(low, pivot - size, size, comp, ud);
  quicksort(pivot + size, high, size, comp, ud);

#undef LESS_THAN_EQUAL
#undef SWAP
#undef SORT2
}

void sort_qsort(void *ptr, size_t count, size_t size, int (*comp)(const void *, const void *, void *ud), void *ud) {
  quicksort((uint8_t *)ptr, (uint8_t *)ptr + (count - 1) * size, size, comp, ud);
}
