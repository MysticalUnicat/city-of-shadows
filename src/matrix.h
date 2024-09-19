#ifndef matrix_h_INCLUDED
#define matrix_h_INCLUDED

#include "memory.h"
#include "math.h"

static inline void matrix_construct(float xx, float yx, float zx, float wx, float xy, float yy, float zy, float wy,
                                       float xz, float yz, float zz, float wz, float xw, float yw, float zw, float ww,
                                       float result[16]) {
  result[0] = xx;
  result[1] = xy;
  result[2] = xz;
  result[3] = xw;
  result[4] = yx;
  result[5] = yy;
  result[6] = yz;
  result[7] = yw;
  result[8] = zx;
  result[9] = zy;
  result[10] = zz;
  result[11] = zw;
  result[12] = wx;
  result[13] = wy;
  result[14] = wz;
  result[15] = ww;
}

static inline void matrix_identity(float result[16]) {
  // clang-format off
  matrix_construct(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
    result
  );
  // clang-format on
}

static inline void matrix_translation(float x, float y, float z, float result[16]) {
  // clang-format off
  matrix_construct(
    1, 0, 0, x,
    0, 1, 0, y,
    0, 0, 1, z,
    0, 0, 0, 1,
    result
  );
  // clang-format on
}

static inline void matrix_scale(float x, float y, float z, float result[16]) {
  // clang-format off
  matrix_construct(
    x, 0, 0, 0,
    0, y, 0, 0,
    0, 0, z, 0,
    0, 0, 0, 1,
    result
  );
  // clang-format on
}

static inline void matrix_rotation(float angle, float x, float y, float z, float result[16]) {
  double magnitude = sqrt(x * x + y * y + z * z);
  if(magnitude < R_MIN) {
    matrix_identity(result);
    return;
  }
  double one_over_magnitude = 1.0 / magnitude;
  double a = angle * 0.01745329251;
  x *= one_over_magnitude;
  y *= one_over_magnitude;
  z *= one_over_magnitude;
  double s = sin(a);
  double c = cos(a);
  double one_minus_c = 1.0 - c;
  double xs = x * s;
  double ys = y * s;
  double zs = z * s;
  double xc = x * one_minus_c;
  double yc = y * one_minus_c;
  double zc = z * one_minus_c;
  double m_xx = (xc * x) + c;
  double m_xy = (xc * y) + zs;
  double m_xz = (xc * z) - ys;
  double m_yx = (yc * x) - zs;
  double m_yy = (yc * y) + c;
  double m_yz = (yc * z) + xs;
  double m_zx = (zc * x) + ys;
  double m_zy = (zc * y) - xs;
  double m_zz = (zc * z) + c;
  // clang-format off
  matrix_construct(
    m_xx, m_yx, m_zx, 0,
    m_xy, m_yy, m_zy, 0,
    m_xz, m_yz, m_zz, 0,
       0,   0,     0, 1,
    result
  );
  // clang-format on
}

static inline void matrix_rotation_x(float angle, float result[16]) {
  float a = angle * 0.01745329251;
  float s = sin(a);
  float c = cos(a);
  // clang-format off
  matrix_construct(
    1, 0,  0, 0,
    0, c, -s, 0,
    0, s,  c, 0,
    0, 0,  0, 1,
    result
  );
  // clang-format on
}

static inline void matrix_rotation_y(float angle, float result[16]) {
  float a = angle * 0.01745329251;
  float s = sin(a);
  float c = cos(a);
  // clang-format off
  matrix_construct(
     c, 0, s, 0,
     0, 1, 0, 0,
    -s, 0, c, 0,
     0, 0, 0, 1,
    result
  );
  // clang-format on
}

static inline void matrix_rotation_z(float angle, float result[16]) {
  float a = angle * 0.01745329251;
  float s = sin(a);
  float c = cos(a);
  // clang-format off
  matrix_construct(
    c, -s, 0, 0,
    s,  c, 0, 0,
    0,  0, 1, 0,
    0,  0, 0, 1,
    result
  );
  // clang-format on
}

static inline void matrix_frustum(float left, float right, float bottom, float top, float znear, float zfar,
                                     float result[16]) {
  float x = (2 * znear) / (right - left);
  float y = (2 * znear) / (top - bottom);
  float a = (right + left) / (right - left);
  float b = (top + bottom) / (top - bottom);
  float c = -(zfar + znear) / (zfar - znear);
  float d = -(2 * zfar * znear) / (zfar - znear);
  // clang-format off
  matrix_construct(
      x, 0,  a,  0,
      0, y,  b,  0,
      0, 0,  c,  d,
      0, 0, -1,  0,
    result
  );
  // clang-format on
}

static inline void matrix_ortho(float left, float right, float bottom, float top, float znear, float zfar,
                                   float result[16]) {
  float x = 2 / (right - left);
  float y = 2 / (top - bottom);
  float z = -2 / (zfar - znear);
  float a = -(right + left) / (right - left);
  float b = -(top + bottom) / (top - bottom);
  float c = -(zfar + znear) / (zfar - znear);
  // clang-format off
  matrix_construct(
      x, 0, 0, a,
      0, y, 0, b,
      0, 0, z, c,
      0, 0, 0, 1,
    result
  );
  // clang-format on
}

static inline void matrix_multiply(const float a[16], const float b[16], float result[16]) {
  float m[16];
  for(uint32_t i = 0; i < 4; i++) {
    float row[4] = {a[0 + i], a[4 + i], a[8 + i], a[12 + i]};
    // clang-format off
    m[ 0 + i] = row[0] * b[ 0 + 0] + row[1] * b[ 0 + 1] + row[2] * b[ 0 + 2] + row[3] * b[ 0 + 3];
    m[ 4 + i] = row[0] * b[ 4 + 0] + row[1] * b[ 4 + 1] + row[2] * b[ 4 + 2] + row[3] * b[ 4 + 3];
    m[ 8 + i] = row[0] * b[ 8 + 0] + row[1] * b[ 8 + 1] + row[2] * b[ 8 + 2] + row[3] * b[ 8 + 3];
    m[12 + i] = row[0] * b[12 + 0] + row[1] * b[12 + 1] + row[2] * b[12 + 2] + row[3] * b[12 + 3];
    // clang-format on
  }
  memory_copy(result, sizeof(float) * 16, m, sizeof(float) * 16);
}

static inline void matrix_translate(float matrix[16], float x, float y, float z) {
  float temp[16];
  matrix_translation(x, y, z, temp);
  matrix_multiply(matrix, temp, matrix);
}

static inline void matrix_rotate(float matrix[16], float angle, float x, float y, float z) {
  float temp[16];
  matrix_rotation(angle, x, y, z, temp);
  matrix_multiply(matrix, temp, matrix);
}

static inline void matrix_rotate_x(float matrix[16], float angle) {
  float temp[16];
  matrix_rotation_x(angle, temp);
  matrix_multiply(matrix, temp, matrix);
}

static inline void matrix_rotate_y(float matrix[16], float angle) {
  float temp[16];
  matrix_rotation_y(angle, temp);
  matrix_multiply(matrix, temp, matrix);
}

static inline void matrix_rotate_z(float matrix[16], float angle) {
  float temp[16];
  matrix_rotation_z(angle, temp);
  matrix_multiply(matrix, temp, matrix);
}

#endif // matrix_h_INCLUDED
