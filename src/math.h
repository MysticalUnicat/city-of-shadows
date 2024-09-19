#ifndef __LIBRARY_CORE_MATH_H__
#define __LIBRARY_CORE_MATH_H__

#include <float.h>
#include <math.h>
#include <stdbool.h>

#ifndef REAL_PRECISION
#define REAL_PRECISION 32
#endif

#if REAL_PRECISION == 32
typedef float R;

#define R_sin sinf
#define R_cos cosf
#define R_fma fmaf
#define R_pow powf
#define R_sqrt sqrtf
#define R_nan nanf
#define R_abs fabsf
#define R_isnan isnan

#define R_EPSILON FLT_EPSILON
#define R_MIN FLT_MIN
#define R_MAX FLT_MAX
#define R_ZERO 0.0f
#define R_ONE 1.0f
#define R_TWO 2.0f
#define R_PI 3.14159265358979323846264338327950288f
#define R_NAN R_nan("")
#define alais_R_INFINITY INFINITY

#elif REAL_PRECISION == 64
typedef double R;

#define R_sin sin
#define R_cos cos
#define R_fma fma
#define R_pow pow

#else
#error "invalid Alias real precision"
#endif

#define R_isqrt(X) R_pow(X, -0.5)

static inline bool R_is_zero(R a) { return R_abs(a) < R_MIN; }

static inline bool R_fuzzy_eq(R a, R b) { return R_abs(a - b) < R_MIN; }

#define MIN_PAIR(A, B) ((A) < (B) ? (A) : (B))
#define MAX_PAIR(A, B) ((A) > (B) ? (A) : (B))

#define min(A, B) MIN_PAIR(A, B)
#define max(A, B) MAX_PAIR(A, B)

#include "generated/pga2d.h"
#include "generated/pga3d.h"

// TODO modify gen program to add these aliases
// something like:
//  --type line:abc a=e1,b=e2,c=e0
//  --type point:xy x=e02,y=e01,1=e12
//  --type direction:xy x=e02,y=e01,0=e12

typedef pga2d_0100 pga2d_Line;
typedef pga2d_0010 pga2d_Point;
typedef pga2d_0010 pga2d_Direction;
typedef pga2d_1010 pga2d_Motor;

typedef pga3d_01000 pga3d_Plane;
typedef pga3d_00100 pga3d_Line;
typedef pga3d_00010 pga3d_Point;
typedef pga3d_00010 pga3d_Direction;
typedef pga3d_10101 pga3d_Motor;

#define pga2d_Motor_IDENTITY ((pga2d_Motor){.one = 1})
#define pga3d_Motor_IDENTITY ((pga3d_Motor){.one = 1})

#define pga2d_line(a, b, c) ((pga2d_Line){.e1 = a, .e2 = b, .e0 = c})
#define pga2d_line_a(l) l.e1
#define pga2d_line_b(l) l.e2
#define pga2d_line_c(l) l.e0

#define pga2d_point(x, y) ((pga2d_Point){.e02 = -x, .e01 = y, .e12 = 1})
#define pga2d_point_x(p) -p.e02
#define pga2d_point_y(p) p.e01

#define pga3d_point(x, y, z) ((pga3d_Point){.e023 = x, .e013 = y, .e012 = z, .e123 = 1})
#define pga3d_point_x(p) p.e023
#define pga3d_point_y(p) p.e013
#define pga3d_point_z(p) p.e012

#define pga2d_direction(x, y) ((pga2d_Direction){.e02 = -x, .e01 = y})
#define pga2d_direction_x(p) -p.e02
#define pga2d_direction_y(p) p.e01

#define pga3d_direction(x, y, z) ((pga3d_Direction){.e023 = x, .e013 = y, .e012 = z})
#define pga3d_direction_x(p) p.e023
#define pga3d_direction_y(p) p.e013
#define pga3d_direction_z(p) p.e012

#define pga2d_UP pga2d_direction(-1, 0)
#define pga2d_DOWN pga2d_direction(1, 0)
#define pga2d_LEFT pga2d_direction(0, -1)
#define pga2d_RIGHT pga2d_direction(0, 1)

#define pga2d_motor(angle, distance, direction) pga2d_mul_mm(pga2d_rotor(angle), pga2d_translator(distance, direction))
#define pga3d_motor(angle, line, distance, direction) pga3d_mul_mm(pga3d_rotor(angle, line), pga3d_translator(distance, direction))
#define pga3d_motor_to(angle, line, point) pga3d_mul_mm(pga3d_rotor(angle, line), pga3d_translator_to(point))

#define pga2d_sandwich(x, y) pga2d_mul(pga2d_mul(y, x), pga2d_reverse(y))
#define pga3d_sandwich(x, y) pga3d_mul(pga3d_mul(y, x), pga3d_reverse(y))

#define pga2d_lerp(x, y, t) pga2d_add(pga2d_mul(pga2d_s(t), y), pga2d_mul(pga2d_sub(pga2d_s(1), pga2d_s(t)), x))
#define pga3d_lerp(x, y, t) pga3d_add(pga3d_mul(pga3d_s(t), y), pga3d_mul(pga3d_sub(pga3d_s(1), pga3d_s(t)), x))

#define pga2d_norm(x) R_sqrt(R_abs(pga2d_mul(x, pga2d_conjugate(x)).one))
#define pga3d_norm(x) R_sqrt(R_abs(pga3d_mul(x, pga3d_conjugate(x)).one))

#define pga2d_inorm(x) pga2d_norm(pga2d_dual(x))
#define pga3d_inorm(x) pga3d_norm(pga3d_dual(x))

#define pga3d_normalize(x) pga3d_mul(pga3d_s(1.0f / pga3d_norm(x)), x)
#define pga3d_inormalize(x) pga3d_mul(pga3d_s(1.0f / pga3d_inorm(x)), x)

#define pga2d_sandwich_vm(x, y) pga2d_grade_1(pga2d_sandwich(pga2d_v(x), pga2d_m(y)))
#define pga2d_sandwich_vb(x, y) pga2d_grade_1(pga2d_sandwich(pga2d_v(x), pga2d_b(y)))
#define pga2d_sandwich_bm(x, y) pga2d_grade_2(pga2d_sandwich(pga2d_b(x), pga2d_m(y)))
#define pga2d_sandwich_bb(x, y) pga2d_grade_2(pga2d_sandwich(pga2d_b(x), pga2d_b(y)))

#define pga3d_sandwich_tm(x, y) pga3d_grade_3(pga3d_sandwich(pga3d_t(x), pga3d_m(y)))

#define pga2d_lerp_b(x, y, t) pga2d_lerp(pga2d_b(x), pga2d_b(y), t)
#define pga2d_lerp_m(x, y, t) pga2d_lerp(pga2d_m(x), pga2d_m(y), t)

#define pga3d_lerp_m(x, y, t) pga3d_lerp(pga3d_m(x), pga3d_m(y), t)

static inline pga2d_Motor pga2d_rotor(float angle) {
  angle /= 2;
  return (pga2d_Motor){.one = cos(angle), .e12 = sin(angle)};
}

static inline pga3d_Motor pga3d_rotor(float angle, pga3d_Line line) {
  angle /= 2;
  float s = sin(angle);
  return (pga3d_Motor){.one = cos(angle), .e01 = 0, .e02 = 0, .e03 = 0, .e12 = line.e12 * s, .e13 = line.e13 * s, .e23 = line.e23 * s, .e0123 = 0};
}

static inline pga2d_Motor pga2d_translator(float distance, pga2d_Direction direction) {
  distance /= 2;
  return (pga2d_Motor){.one = 1, .e02 = direction.e02 * distance, .e01 = direction.e01 * distance, .e12 = R_ZERO};
}

static inline pga3d_Motor pga3d_translator(float distance, pga3d_Direction direction) {
  distance /= 2;
  return (pga3d_Motor){
      .one = 1, .e01 = direction.e023 * distance, .e02 = direction.e013 * distance, .e03 = direction.e012 * distance, .e12 = 0, .e13 = 0, .e23 = 0, .e0123 = 0};
}

static inline pga2d_Motor pga2d_translator_to(pga2d_Point point) {
  float distance = 0.5f;
  return (pga2d_Motor){.one = 1, .e02 = point.e02 * distance, .e01 = point.e01 * distance, .e12 = R_ZERO};
}

static inline pga3d_Motor pga3d_translator_to(pga3d_Point point) {
  pga3d_Line line = pga3d_grade_2(pga3d_mul(pga3d_t(point), pga3d_negate_t(pga3d_point(0, 0, 0))));
  return (pga3d_Motor){.one = 1, .e01 = line.e01 * 0.5f, .e02 = line.e02 * 0.5f, .e03 = line.e03 * 0.5f, .e12 = 0, .e13 = 0, .e23 = 0, .e0123 = 0};
}

#endif

