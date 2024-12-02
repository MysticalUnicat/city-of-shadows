#ifndef linear_allocator_h_INCLUDED
#define linear_allocator_h_INCLUDED

struct LinearAllocator {
  uint32_t    num_pages;
  uint8_t     * * pages;
};

#endif // linear_allocator_h_INCLUDED
