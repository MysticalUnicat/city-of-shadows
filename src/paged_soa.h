#ifndef __LIBRARY_CORE_DATA_STRUCTURE_PAGED_SOA_H_
#define __LIBRARY_CORE_DATA_STRUCTURE_PAGED_SOA_H_

#include <stdbool.h>
#include <stdalign.h>

#include "memory.h"

#define PAGED_SOA_PAGE_SIZE (1 << 16)
#define PAGED_SOA_PAGE_ALIGNMENT 16

typedef struct PagedSOA {
  uint32_t    length;
  uint32_t    num_columns;
  uint32_t    rows_per_page;
  uint32_t  * size_offset;
  uint32_t    num_pages;
  uint8_t * * pages;
} PagedSOA;

static inline bool PagedSOA_initialize(PagedSOA * soa, uint32_t prefix_size, uint32_t num_columns, const size_t * sizes) {
  soa->length = 0;
  soa->num_pages = 0;
  soa->num_columns = num_columns;
  soa->size_offset = memory_alloc(sizeof(*soa->size_offset) * num_columns, alignof(*soa->size_offset));
  if(soa->size_offset == NULL) {
    return false;
  }
  
  soa->pages = NULL;

  uint32_t row_size = 0;
  for(uint32_t i = 0; i < soa->num_columns; i++) {
    row_size += sizes[i];
  }

  soa->rows_per_page = (PAGED_SOA_PAGE_SIZE - prefix_size) / row_size;

  uint32_t offset = prefix_size;

  for(uint32_t i = 0; i < soa->num_columns; i++) {
    soa->size_offset[i] = (sizes[i] << 16) | offset;
    offset += sizes[i] * soa->rows_per_page;
  }

  return true;
}

static inline void PagedSOA_free(PagedSOA * soa) {
  memory_free(soa->size_offset, sizeof(*soa->size_offset) * soa->num_columns, alignof(*soa->size_offset));
  for(uint32_t i = 0; i < soa->num_pages; i++) {
    memory_free(soa->pages[i], PAGED_SOA_PAGE_SIZE, PAGED_SOA_PAGE_ALIGNMENT);
  }
  memory_free(soa->pages, sizeof(*soa->pages) * soa->num_pages, alignof(*soa->pages));
  memory_clear(soa, sizeof(*soa));
}

static inline bool PagedSOA_set_capacity(PagedSOA * soa, uint32_t new_capacity) {
  uint32_t new_num_pages = (new_capacity + soa->rows_per_page - 1) / soa->rows_per_page;
  if(new_num_pages > soa->num_pages) {
    soa->pages = memory_realloc(soa->pages, sizeof(*soa->pages) * soa->num_pages, sizeof(*soa->pages) * new_num_pages, alignof(*soa->pages));
    while(soa->num_pages < new_num_pages) {
      uint8_t * page = memory_alloc(PAGED_SOA_PAGE_SIZE, PAGED_SOA_PAGE_ALIGNMENT);
      memory_clear(page, soa->size_offset[0] & 0xFFFF);
      soa->pages[soa->num_pages++] = page;
    }
    soa->num_pages = new_num_pages;
  }
  return true;
}

static inline bool PagedSOA_space_for(PagedSOA * soa, uint32_t count) {
  uint32_t new_num_pages = (soa->length + count + soa->rows_per_page - 1) / soa->rows_per_page;
  if(new_num_pages > soa->num_pages) {
    return PagedSOA_set_capacity(soa, soa->length + count);
  }
  return true;
}

static inline uint32_t PagedSOA_push(PagedSOA * soa) {
  uint32_t row = soa->length++;
  uint32_t page = row / soa->rows_per_page;
  uint32_t index = row % soa->rows_per_page;
  return (page << 16) | index;
}

static inline void * PagedSOA_page(const PagedSOA * soa, uint32_t code) {
  uint32_t page = code >> 16;
  return soa->pages[page];
}

static inline void PagedSOA_decode_code(const PagedSOA * soa, uint32_t code, uint32_t * page, uint32_t * index) {
  (void)soa;
  *page = code >> 16;
  *index = code & 0xFFFF;
}

static inline void PagedSOA_decode_column(const PagedSOA * soa, uint32_t column, uint32_t * size, uint32_t * offset) {
  *size = soa->size_offset[column] >> 16;
  *offset = soa->size_offset[column] & 0xFFFF;
}

static inline const void * PagedSOA_raw_read(const PagedSOA * soa, uint32_t page, uint32_t index, uint32_t size, uint32_t offset) {
  return soa->pages[page] + (offset + size * index);
}

static inline const void * PagedSOA_read_first(const PagedSOA * soa, uint32_t page, uint32_t column) {
  uint32_t size, offset;
  PagedSOA_decode_column(soa, column, &size, &offset);
  return PagedSOA_raw_read(soa, page, 0, size, offset);
}

static inline const void * PagedSOA_read(const PagedSOA * soa, uint32_t code, uint32_t column) {
  uint32_t page, index, size, offset;
  PagedSOA_decode_code(soa, code, &page, &index);
  PagedSOA_decode_column(soa, column, &size, &offset);
  return PagedSOA_raw_read(soa, page, index, size, offset);
}

static inline void * PagedSOA_raw_write(PagedSOA * soa, uint32_t page, uint32_t index, uint32_t size, uint32_t offset) {
  return soa->pages[page] + (offset + size * index);
}

static inline void * PagedSOA_write_first(PagedSOA * soa, uint32_t page, uint32_t column) {
  uint32_t size, offset;
  PagedSOA_decode_column(soa, column, &size, &offset);
  return PagedSOA_raw_write(soa, page, 0, size, offset);
}

static inline void * PagedSOA_write(PagedSOA * soa, uint32_t code, uint32_t column) {
  uint32_t page, index, size, offset;
  PagedSOA_decode_code(soa, code, &page, &index);
  PagedSOA_decode_column(soa, column, &size, &offset);
  return PagedSOA_raw_write(soa, page, index, size, offset);
}

#endif
