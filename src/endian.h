#pragma once

#include <stdint.h>

static inline void endian_swap_16(uint8_t bytes[2]) {
  uint8_t temp[2] = { bytes[1], bytes[0] };
  bytes = temp;
}

static inline void endian_swap_32(uint8_t bytes[4]) {
  uint8_t temp[4] = { bytes[3], bytes[2], bytes[1], bytes[0] };
  bytes = temp;
}

static inline void endian_swap_64(uint8_t bytes[8]) {
  uint8_t temp[8] = { bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0] };
  bytes = temp;
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define endian_little_16(x)
#define endian_little_32(x)
#define endian_little_64(x)
#define endian_big_16(x) endian_swap_16((uint8_t *)(x))
#define endian_big_32(x) endian_swap_32((uint8_t *)(x))
#define endian_big_64(x) endian_swap_64((uint8_t *)(x))
#else
#define endian_little_16(x) endian_swap_16((uint8_t *)(x))
#define endian_little_32(x) endian_swap_32((uint8_t *)(x))
#define endian_little_64(x) endian_swap_64((uint8_t *)(x))
#define endian_big_16(x)
#define endian_big_32(x)
#define endian_big_64(x)
#endif

static inline void endian_little_u16(uint16_t * x) {
  endian_little_16(x);
}

static inline void endian_little_i16(int16_t * x) {
  endian_little_16(x);
}

static inline void endian_little_u32(uint32_t * x) {
  endian_little_32(x);
}

static inline void endian_little_i32(int32_t * x) {
  endian_little_32(x);
}

static inline void endian_little_float(float * x) {
  endian_little_32(x);
}

static inline void endian_little_u64(uint64_t * x) {
  endian_little_64(x);
}

static inline void endian_little_i64(int64_t * x) {
  endian_little_64(x);
}

static inline void endian_big_u16(uint16_t * x) {
  endian_big_16(x);
}

static inline void endian_big_i16(int16_t * x) {
  endian_big_16(x);
}

static inline void endian_big_u32(uint32_t * x) {
  endian_big_32(x);
}

static inline void endian_big_i32(int32_t * x) {
  endian_big_32(x);
}

static inline void endian_big_float(float * x) {
  endian_big_32(x);
}

static inline void endian_big_u64(uint64_t * x) {
  endian_big_64(x);
}

static inline void endian_big_i64(int64_t * x) {
  endian_big_64(x);
}

#define endian_little(x) \
  _Generic((x), \
    uint16_t *: endian_little_u16, \
     int16_t *: endian_little_i16, \
    uint32_t *: endian_little_u32, \
     int32_t *: endian_little_i32, \
       float *: endian_little_float, \
    uint64_t *: endian_little_u64, \
     int64_t *: endian_little_i64 \
  )(x)

#define endian_big(x) \
  _Generic((x), \
    uint16_t *: endian_big_u16, \
     int16_t *: endian_big_i16, \
    uint32_t *: endian_big_u32, \
     int32_t *: endian_big_i32, \
       float *: endian_big_float, \
    uint64_t *: endian_big_u64, \
     int64_t *: endian_big_i64 \
  )(x)


