#ifndef color_h_INCLUDED
#define color_h_INCLUDED

#include <stdint.h>

#include "math.h"

typedef struct Color {
  R r;
  R g;
  R b;
  R a;
} Color;

extern const Color Color_WHITE;
extern const Color Color_GRAY;
extern const Color Color_BLACK;
extern const Color Color_TRANSPARENT_BLACK;

#define u8_to_R(X) ((R)(X) / (R)255)
#define R_to_u8(X) (max(0, min(1, (R)(X))) * (R)255)

static inline Color Color_from_rgb_u8(uint8_t r, uint8_t g, uint8_t b) {
  return (Color) { u8_to_R(r), u8_to_R(g), u8_to_R(b), R_ONE };
}

static inline uint32_t Color_to_rgba_u8_packed(Color c) {
  union {
    uint32_t i;
    uint8_t b[4];
  } u;
  u.b[0] = R_to_u8(c.r);
  u.b[1] = R_to_u8(c.g);
  u.b[2] = R_to_u8(c.b);
  u.b[3] = R_to_u8(c.a);
  return u.i;
}

#endif // color_h_INCLUDED
