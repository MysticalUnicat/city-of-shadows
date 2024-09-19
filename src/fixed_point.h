#ifndef fixed_point_h_INCLUDED
#define fixed_point_h_INCLUDED

#include <stdint.h>

#include "cpp_type.h"

typedef int8_t   FixedPoint4;
typedef int16_t  FixedPoint8;
typedef int32_t  FixedPoint16;

typedef uint8_t  FixedPointUnorm8;
typedef uint16_t FixedPointUnorm16;
typedef uint32_t FixedPointUnorm32;

#define CPP_TYPE_CONVERT_float_FixedPoint4(A) (FixedPoint4)((A) * 16.0f)
#define CPP_TYPE_CONVERT_float_FixedPoint8(A) (FixedPoint8)((A) * 256.0f)
#define CPP_TYPE_CONVERT_float_FixedPoint16(A) (FixedPoint16)((A) * 65536.0f)
#define CPP_TYPE_CONVERT_float_FixedPointUnorm16(A) (FixedPointUnorm16)((A) * 65536.0f)

#define CPP_TYPE_CONVERT_FixedPoint4_float(A) ((float)(A) / 16.0f)
#define CPP_TYPE_CONVERT_FixedPoint8_float(A) ((float)(A) / 256.0f)
#define CPP_TYPE_CONVERT_FixedPoint16_float(A) ((float)(A) / 65536.0f)
#define CPP_TYPE_CONVERT_FixedPointUnorm16_float(A) ((float)(A) / 65536.0f)

#endif // fixed_point_h_INCLUDED
