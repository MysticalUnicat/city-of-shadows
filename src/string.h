#ifndef __LIBRARY_STRING_H__
#define __LIBRARY_STRING_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef const char * CStr;
typedef uint8_t * MStr;
typedef struct {
  uint32_t length;
  const char * base;
} AStr;

#define STRING_MK_GENERIC_1(PREFIX, A, ...) _Generic((A), MStr: PREFIX ## _m, AStr: PREFIX ## _a, default: PREFIX ## _c)(A, ## __VA_ARGS__)

int string_size_m(MStr m);
int string_size_c(CStr c);
int string_size_a(AStr a);
#define string_size(s) _Generic((s), MStr: string_size_m, AStr: string_size_a, default: string_size_c)(s)

bool string_equals(CStr a, CStr b);

int string_compare(CStr a, CStr b);
int string_compare_length(CStr a, CStr b, size_t length);

CStr string_find(CStr s, int n);

bool string_endswith_m(MStr m, CStr n);
bool string_endswith_c(CStr c, CStr n);
bool string_endswith_a(AStr a, CStr n);
#define string_endswith(s, n) STRING_MK_GENERIC_1(string_endswith, s, n)

CStr string_find_reverse_m(MStr m, int n);
CStr string_find_reverse_c(CStr c, int n);
AStr string_find_reverse_a(AStr a, int n);
#define string_find_reverse(s, n) _Generic((s), MStr: string_find_reverse_m, default: string_find_reverse_c)(s, n)

uint64_t string_hash(CStr s);

MStr string_allocate(uint32_t capacity);

MStr string_clone_m(MStr s);
MStr string_clone_c(CStr s);
MStr string_clone_a(AStr s);
#define string_clone(s) \
  _Generic((s), MStr: string_clone_m, \
                AStr: string_clone_a, \
             default: string_clone_c)(s)

MStr string_clone_length_m(MStr s, size_t length);
MStr string_clone_length_c(CStr s, size_t length);
MStr string_clone_length_a(AStr s, size_t length);
#define string_clone_length(s, n) \
  _Generic((s), MStr: string_clone_length_m, \
                AStr: string_clone_length_a, \
             default: string_clone_length_c)(s, n)

MStr string_concatenate_mm(MStr left, MStr right);
MStr string_concatenate_mc(MStr left, CStr right);
MStr string_concatenate_ma(MStr left, AStr right);
MStr string_concatenate_cm(CStr left, MStr right);
MStr string_concatenate_cc(CStr left, CStr right);
MStr string_concatenate_ca(CStr left, AStr right);
MStr string_concatenate_am(AStr left, MStr right);
MStr string_concatenate_ac(AStr left, CStr right);
MStr string_concatenate_aa(AStr left, AStr right);
#define string_concatenate(left, right) \
  _Generic((left), \
       MStr: _Generic((right), MStr: string_concatenate_mm, \
                               AStr: string_concatenate_ma, \
                            default: string_concatenate_mc), \
       AStr: _Generic((right), MStr: string_concatenate_am, \
                               AStr: string_concatenate_aa, \
                            default: string_concatenate_ac), \
    default: _Generic((right), MStr: string_concatenate_cm, \
                               AStr: string_concatenate_ca, \
                            default: string_concatenate_cc))(left, right) \

MStr string_concatenate_length_mm(MStr left, MStr right, size_t length);
MStr string_concatenate_length_mc(MStr left, CStr right, size_t length);
MStr string_concatenate_length_ma(MStr left, AStr right, size_t length);
MStr string_concatenate_length_cm(CStr left, MStr right, size_t length);
MStr string_concatenate_length_cc(CStr left, CStr right, size_t length);
MStr string_concatenate_length_ca(CStr left, AStr right, size_t length);
MStr string_concatenate_length_am(AStr left, MStr right, size_t length);
MStr string_concatenate_length_ac(AStr left, CStr right, size_t length);
MStr string_concatenate_length_aa(AStr left, AStr right, size_t length);
#define string_concatenate_length(left, right, length) \
  _Generic((left), \
       MStr: _Generic((right), MStr: string_concatenate_length_mm, \
                               AStr: string_concatenate_length_ma, \
                            default: string_concatenate_length_mc), \
       AStr: _Generic((right), MStr: string_concatenate_length_am, \
                               AStr: string_concatenate_length_aa, \
                            default: string_concatenate_length_ac), \
    default: _Generic((right), MStr: string_concatenate_length_cm, \
                               AStr: string_concatenate_length_ca, \
                            default: string_concatenate_length_cc))(left, right, length) \

MStr string_append_m(MStr left, uint8_t byte);
MStr string_append_c(CStr left, uint8_t byte);
MStr string_append_a(AStr left, uint8_t byte);
#define string_append(left, byte) \
  _Generic((left), \
       MStr: string_append_m, \
       AStr: string_append_a, \
    default: string_append_c)(left, byte)

void string_clear(MStr s);

void string_free(MStr s);

#define STRING_HASH_INIT     11ull
#define STRING_HASH_MULTIPLY 17ull

#define STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, INDEX, HASH) (S[INDEX < S_SIZE ? S_SIZE - 1 - INDEX : 0] ^ HASH) * STRING_HASH_MULTIPLY

#define STRING_COMPILE_TIME_HASH_63(S, S_SIZE, HASH) HASH
#define STRING_COMPILE_TIME_HASH_62(S, S_SIZE, HASH) (62 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 62, STRING_COMPILE_TIME_HASH_63(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_61(S, S_SIZE, HASH) (61 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 61, STRING_COMPILE_TIME_HASH_62(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_60(S, S_SIZE, HASH) (60 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 60, STRING_COMPILE_TIME_HASH_61(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_59(S, S_SIZE, HASH) (59 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 59, STRING_COMPILE_TIME_HASH_60(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_58(S, S_SIZE, HASH) (58 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 58, STRING_COMPILE_TIME_HASH_59(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_57(S, S_SIZE, HASH) (57 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 57, STRING_COMPILE_TIME_HASH_58(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_56(S, S_SIZE, HASH) (56 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 56, STRING_COMPILE_TIME_HASH_57(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_55(S, S_SIZE, HASH) (55 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 55, STRING_COMPILE_TIME_HASH_56(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_54(S, S_SIZE, HASH) (54 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 54, STRING_COMPILE_TIME_HASH_55(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_53(S, S_SIZE, HASH) (53 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 53, STRING_COMPILE_TIME_HASH_54(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_52(S, S_SIZE, HASH) (52 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 52, STRING_COMPILE_TIME_HASH_53(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_51(S, S_SIZE, HASH) (51 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 51, STRING_COMPILE_TIME_HASH_52(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_50(S, S_SIZE, HASH) (50 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 50, STRING_COMPILE_TIME_HASH_51(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_49(S, S_SIZE, HASH) (49 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 49, STRING_COMPILE_TIME_HASH_50(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_48(S, S_SIZE, HASH) (48 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 48, STRING_COMPILE_TIME_HASH_49(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_47(S, S_SIZE, HASH) (47 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 47, STRING_COMPILE_TIME_HASH_48(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_46(S, S_SIZE, HASH) (46 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 46, STRING_COMPILE_TIME_HASH_47(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_45(S, S_SIZE, HASH) (45 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 45, STRING_COMPILE_TIME_HASH_46(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_44(S, S_SIZE, HASH) (44 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 44, STRING_COMPILE_TIME_HASH_45(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_43(S, S_SIZE, HASH) (43 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 43, STRING_COMPILE_TIME_HASH_44(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_42(S, S_SIZE, HASH) (42 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 42, STRING_COMPILE_TIME_HASH_43(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_41(S, S_SIZE, HASH) (41 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 41, STRING_COMPILE_TIME_HASH_42(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_40(S, S_SIZE, HASH) (40 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 40, STRING_COMPILE_TIME_HASH_41(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_39(S, S_SIZE, HASH) (39 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 39, STRING_COMPILE_TIME_HASH_40(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_38(S, S_SIZE, HASH) (38 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 38, STRING_COMPILE_TIME_HASH_39(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_37(S, S_SIZE, HASH) (37 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 37, STRING_COMPILE_TIME_HASH_38(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_36(S, S_SIZE, HASH) (36 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 36, STRING_COMPILE_TIME_HASH_37(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_35(S, S_SIZE, HASH) (35 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 35, STRING_COMPILE_TIME_HASH_36(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_34(S, S_SIZE, HASH) (34 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 34, STRING_COMPILE_TIME_HASH_35(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_33(S, S_SIZE, HASH) (33 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 33, STRING_COMPILE_TIME_HASH_34(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_32(S, S_SIZE, HASH) (32 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 32, STRING_COMPILE_TIME_HASH_33(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_31(S, S_SIZE, HASH) (31 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 31, STRING_COMPILE_TIME_HASH_32(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_30(S, S_SIZE, HASH) (30 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 30, STRING_COMPILE_TIME_HASH_31(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_29(S, S_SIZE, HASH) (29 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 29, STRING_COMPILE_TIME_HASH_30(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_28(S, S_SIZE, HASH) (28 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 28, STRING_COMPILE_TIME_HASH_29(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_27(S, S_SIZE, HASH) (27 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 27, STRING_COMPILE_TIME_HASH_28(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_26(S, S_SIZE, HASH) (26 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 26, STRING_COMPILE_TIME_HASH_27(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_25(S, S_SIZE, HASH) (25 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 25, STRING_COMPILE_TIME_HASH_26(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_24(S, S_SIZE, HASH) (24 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 24, STRING_COMPILE_TIME_HASH_25(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_23(S, S_SIZE, HASH) (23 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 23, STRING_COMPILE_TIME_HASH_24(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_22(S, S_SIZE, HASH) (22 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 22, STRING_COMPILE_TIME_HASH_23(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_21(S, S_SIZE, HASH) (21 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 21, STRING_COMPILE_TIME_HASH_22(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_20(S, S_SIZE, HASH) (20 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 20, STRING_COMPILE_TIME_HASH_21(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_19(S, S_SIZE, HASH) (19 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 19, STRING_COMPILE_TIME_HASH_20(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_18(S, S_SIZE, HASH) (18 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 18, STRING_COMPILE_TIME_HASH_19(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_17(S, S_SIZE, HASH) (17 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 17, STRING_COMPILE_TIME_HASH_18(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_16(S, S_SIZE, HASH) (16 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 16, STRING_COMPILE_TIME_HASH_17(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_15(S, S_SIZE, HASH) (15 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 15, STRING_COMPILE_TIME_HASH_16(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_14(S, S_SIZE, HASH) (14 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 14, STRING_COMPILE_TIME_HASH_15(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_13(S, S_SIZE, HASH) (13 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 13, STRING_COMPILE_TIME_HASH_14(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_12(S, S_SIZE, HASH) (12 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 12, STRING_COMPILE_TIME_HASH_13(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_11(S, S_SIZE, HASH) (11 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 11, STRING_COMPILE_TIME_HASH_12(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_10(S, S_SIZE, HASH) (10 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE, 10, STRING_COMPILE_TIME_HASH_11(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_9( S, S_SIZE, HASH) ( 9 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE,  9, STRING_COMPILE_TIME_HASH_10(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_8( S, S_SIZE, HASH) ( 8 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE,  8,  STRING_COMPILE_TIME_HASH_9(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_7( S, S_SIZE, HASH) ( 7 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE,  7,  STRING_COMPILE_TIME_HASH_8(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_6( S, S_SIZE, HASH) ( 6 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE,  6,  STRING_COMPILE_TIME_HASH_7(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_5( S, S_SIZE, HASH) ( 5 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE,  5,  STRING_COMPILE_TIME_HASH_6(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_4( S, S_SIZE, HASH) ( 4 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE,  4,  STRING_COMPILE_TIME_HASH_5(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_3( S, S_SIZE, HASH) ( 3 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE,  3,  STRING_COMPILE_TIME_HASH_4(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_2( S, S_SIZE, HASH) ( 2 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE,  2,  STRING_COMPILE_TIME_HASH_3(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH_1( S, S_SIZE, HASH) ( 1 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE,  1,  STRING_COMPILE_TIME_HASH_2(S, S_SIZE, HASH)) : HASH)
#define STRING_COMPILE_TIME_HASH(   S, S_SIZE, HASH) ( 0 < S_SIZE ? STRING_COMPILE_TIME_HASH_FUNCTION(S, S_SIZE,  0,  STRING_COMPILE_TIME_HASH_1(S, S_SIZE, HASH)) : HASH)

#define STRING_CONSTANT(S) ((uint64_t)(STRING_COMPILE_TIME_HASH(S, sizeof(S) - 1, STRING_HASH_INIT)))

#endif
