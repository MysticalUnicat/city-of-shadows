#ifndef cpp_type_h_INCLUDED
#define cpp_type_h_INCLUDED

#include "cpp.h"

#define CPP_TYPE_CONVERT(A, B) CPP_CAT4(CPP_TYPE_CONVERT_, A, _, B)

#define CPP_TYPE_CONVERT_float_float(A) (A)
#define CPP_TYPE_CONVERT_uint8_t_uint8_t(A) (A)
#define CPP_TYPE_CONVERT_uint16_t_uint16_t(A) (A)
#define CPP_TYPE_CONVERT_uint8_t_float(A) (float)(A)
#define CPP_TYPE_CONVERT_uint16_t_float(A) (float)(A)
#define CPP_TYPE_CONVERT_float_uint8_t(A) (uint8_t)(A)
#define CPP_TYPE_CONVERT_float_uint16_t(A) (uint16_t)(A)

#endif // cpp_type_h_INCLUDED
