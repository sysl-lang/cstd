/* <stdbit.h> from sh.sysl.cstd, as ISO C23 section 7.18 specifies it: fourteen families of bit
 * questions over the unsigned integer types, a function per type under the suffixes _uc _us _ui _ul
 * _ull, and a type-generic macro per family that picks the function by the argument's type.
 *
 * The argument of a type-generic macro must have type unsigned char, unsigned short, unsigned int,
 * unsigned long or unsigned long long: any other type (a signed one, plain char, bool) matches no
 * association and is a compile-time error.
 *
 * stdc_bit_ceil of a value above the largest power of two its type holds is undefined in C; here it
 * answers 0. */
#ifndef CSTD_STDBIT_H
#define CSTD_STDBIT_H

#include <stdbool.h>

#define __STDC_VERSION_STDBIT_H__ 202311L

#define __STDC_ENDIAN_LITTLE__ __ORDER_LITTLE_ENDIAN__
#define __STDC_ENDIAN_BIG__ __ORDER_BIG_ENDIAN__
#define __STDC_ENDIAN_NATIVE__ __BYTE_ORDER__

#ifdef __cplusplus
extern "C" {
#endif

/* One family of functions answering an unsigned int, then one answering bool, then one answering the
 * argument's own type. */
#define CSTD_STDBIT_COUNT_FAMILY(name)                 \
    unsigned int name##_uc(unsigned char value);       \
    unsigned int name##_us(unsigned short value);      \
    unsigned int name##_ui(unsigned int value);        \
    unsigned int name##_ul(unsigned long value);       \
    unsigned int name##_ull(unsigned long long value)

#define CSTD_STDBIT_SAME_FAMILY(name)                  \
    unsigned char name##_uc(unsigned char value);      \
    unsigned short name##_us(unsigned short value);    \
    unsigned int name##_ui(unsigned int value);        \
    unsigned long name##_ul(unsigned long value);      \
    unsigned long long name##_ull(unsigned long long value)

CSTD_STDBIT_COUNT_FAMILY(stdc_leading_zeros);
CSTD_STDBIT_COUNT_FAMILY(stdc_leading_ones);
CSTD_STDBIT_COUNT_FAMILY(stdc_trailing_zeros);
CSTD_STDBIT_COUNT_FAMILY(stdc_trailing_ones);
CSTD_STDBIT_COUNT_FAMILY(stdc_first_leading_zero);
CSTD_STDBIT_COUNT_FAMILY(stdc_first_leading_one);
CSTD_STDBIT_COUNT_FAMILY(stdc_first_trailing_zero);
CSTD_STDBIT_COUNT_FAMILY(stdc_first_trailing_one);
CSTD_STDBIT_COUNT_FAMILY(stdc_count_zeros);
CSTD_STDBIT_COUNT_FAMILY(stdc_count_ones);
CSTD_STDBIT_COUNT_FAMILY(stdc_bit_width);
CSTD_STDBIT_SAME_FAMILY(stdc_bit_floor);
CSTD_STDBIT_SAME_FAMILY(stdc_bit_ceil);

bool stdc_has_single_bit_uc(unsigned char value);
bool stdc_has_single_bit_us(unsigned short value);
bool stdc_has_single_bit_ui(unsigned int value);
bool stdc_has_single_bit_ul(unsigned long value);
bool stdc_has_single_bit_ull(unsigned long long value);

#ifdef __cplusplus
}
#endif

#ifndef __cplusplus

/* The call is outside the _Generic, whose controlling expression is not evaluated: the argument is
 * evaluated exactly once. */
#define CSTD_STDBIT_GENERIC(name, value)               \
    _Generic((value),                                  \
        unsigned char: name##_uc,                      \
        unsigned short: name##_us,                     \
        unsigned int: name##_ui,                       \
        unsigned long: name##_ul,                      \
        unsigned long long: name##_ull)(value)

#define stdc_leading_zeros(value) CSTD_STDBIT_GENERIC(stdc_leading_zeros, value)
#define stdc_leading_ones(value) CSTD_STDBIT_GENERIC(stdc_leading_ones, value)
#define stdc_trailing_zeros(value) CSTD_STDBIT_GENERIC(stdc_trailing_zeros, value)
#define stdc_trailing_ones(value) CSTD_STDBIT_GENERIC(stdc_trailing_ones, value)
#define stdc_first_leading_zero(value) CSTD_STDBIT_GENERIC(stdc_first_leading_zero, value)
#define stdc_first_leading_one(value) CSTD_STDBIT_GENERIC(stdc_first_leading_one, value)
#define stdc_first_trailing_zero(value) CSTD_STDBIT_GENERIC(stdc_first_trailing_zero, value)
#define stdc_first_trailing_one(value) CSTD_STDBIT_GENERIC(stdc_first_trailing_one, value)
#define stdc_count_zeros(value) CSTD_STDBIT_GENERIC(stdc_count_zeros, value)
#define stdc_count_ones(value) CSTD_STDBIT_GENERIC(stdc_count_ones, value)
#define stdc_has_single_bit(value) CSTD_STDBIT_GENERIC(stdc_has_single_bit, value)
#define stdc_bit_width(value) CSTD_STDBIT_GENERIC(stdc_bit_width, value)
#define stdc_bit_floor(value) CSTD_STDBIT_GENERIC(stdc_bit_floor, value)
#define stdc_bit_ceil(value) CSTD_STDBIT_GENERIC(stdc_bit_ceil, value)

#endif

#endif
