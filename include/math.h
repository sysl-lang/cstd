/* <math.h> from sh.sysl.cstd: the C99/C17 mathematics in double and float everywhere, and in long
 * double where long double is a double (arm64 macOS, 32-bit Arm). Each function is ported from
 * musl's libm (itself FreeBSD's msun and Arm's optimized-routines) into sysl, rounding to nearest.
 *
 * Errors are reported through errno only: math_errhandling is MATH_ERRNO. A domain error sets EDOM
 * and answers NaN; a pole error and an overflow set ERANGE and answer an infinity. An underflow
 * answers the subnormal or zero result and leaves errno alone, which POSIX allows. The exception
 * flags are raised as the arithmetic underneath raises them, and nothing promises which. */
#ifndef CSTD_MATH_H
#define CSTD_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Every target cstd builds for evaluates float in float and double in double (FLT_EVAL_METHOD 0). */
typedef float float_t;
typedef double double_t;

#define HUGE_VAL  __builtin_huge_val()
#define HUGE_VALF __builtin_huge_valf()
#define HUGE_VALL __builtin_huge_vall()
#define INFINITY  __builtin_inff()
#define NAN       __builtin_nanf("")

#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

#define FP_ILOGB0   (-2147483647 - 1)
#define FP_ILOGBNAN (-2147483647 - 1)

#define MATH_ERRNO     1
#define MATH_ERREXCEPT 2
#define math_errhandling MATH_ERRNO

/* The classification and comparison macros are the compiler's own generic operations, so they take
 * any real-floating type and need nothing from the library. The comparisons are quiet: a NaN
 * operand answers 0 without the invalid exception a relational operator would raise. */
#define fpclassify(x) __builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO, (x))
#define isfinite(x)   __builtin_isfinite(x)
#define isinf(x)      __builtin_isinf(x)
#define isnan(x)      __builtin_isnan(x)
#define isnormal(x)   __builtin_isnormal(x)
#define signbit(x)    __builtin_signbit(x)

#define isgreater(x, y)      __builtin_isgreater((x), (y))
#define isgreaterequal(x, y) __builtin_isgreaterequal((x), (y))
#define isless(x, y)         __builtin_isless((x), (y))
#define islessequal(x, y)    __builtin_islessequal((x), (y))
#define islessgreater(x, y)  __builtin_islessgreater((x), (y))
#define isunordered(x, y)    __builtin_isunordered((x), (y))

/* [XSI] constants, correctly rounded at each width. */
#define M_E        2.71828182845904523536028747135266250
#define M_EGAMMA   0.577215664901532860606512090082402431
#define M_LOG2E    1.44269504088896340735992468100189214
#define M_LOG10E   0.434294481903251827651128918916605082
#define M_LN2      0.693147180559945309417232121458176568
#define M_LN10     2.30258509299404568401799145468436421
#define M_PHI      1.61803398874989484820458683436563812
#define M_PI       3.14159265358979323846264338327950288
#define M_PI_2     1.57079632679489661923132169163975144
#define M_PI_4     0.785398163397448309615660845819875721
#define M_1_PI     0.318309886183790671537767526745028724
#define M_1_SQRTPI 0.564189583547756286948079451560772586
#define M_2_PI     0.636619772367581343075535053490057448
#define M_2_SQRTPI 1.12837916709551257389615890312154517
#define M_SQRT2    1.41421356237309504880168872420969808
#define M_SQRT3    1.73205080756887729352744634150587237
#define M_SQRT1_2  0.707106781186547524400844362104849039
#define M_SQRT1_3  0.577350269189625764509148780501957456

#define M_El        2.71828182845904523536028747135266250L
#define M_EGAMMAl   0.577215664901532860606512090082402431L
#define M_LOG2El    1.44269504088896340735992468100189214L
#define M_LOG10El   0.434294481903251827651128918916605082L
#define M_LN2l      0.693147180559945309417232121458176568L
#define M_LN10l     2.30258509299404568401799145468436421L
#define M_PHIl      1.61803398874989484820458683436563812L
#define M_PIl       3.14159265358979323846264338327950288L
#define M_PI_2l     1.57079632679489661923132169163975144L
#define M_PI_4l     0.785398163397448309615660845819875721L
#define M_1_PIl     0.318309886183790671537767526745028724L
#define M_1_SQRTPIl 0.564189583547756286948079451560772586L
#define M_2_PIl     0.636619772367581343075535053490057448L
#define M_2_SQRTPIl 1.12837916709551257389615890312154517L
#define M_SQRT2l    1.41421356237309504880168872420969808L
#define M_SQRT3l    1.73205080756887729352744634150587237L
#define M_SQRT1_2l  0.707106781186547524400844362104849039L
#define M_SQRT1_3l  0.577350269189625764509148780501957456L

double acos(double x);
double asin(double x);
double atan(double x);
double atan2(double y, double x);
double cos(double x);
double sin(double x);
double tan(double x);
double acosh(double x);
double asinh(double x);
double atanh(double x);
double cosh(double x);
double sinh(double x);
double tanh(double x);
double exp(double x);
double exp2(double x);
double expm1(double x);
double frexp(double x, int *exp);
int ilogb(double x);
double ldexp(double x, int exp);
double log(double x);
double log10(double x);
double log1p(double x);
double log2(double x);
double logb(double x);
double modf(double x, double *iptr);
double scalbn(double x, int n);
double scalbln(double x, long n);
double cbrt(double x);
double fabs(double x);
double hypot(double x, double y);
double pow(double x, double y);
double sqrt(double x);
double erf(double x);
double erfc(double x);
double lgamma(double x);
double tgamma(double x);
double ceil(double x);
double floor(double x);
double nearbyint(double x);
double rint(double x);
long lrint(double x);
long long llrint(double x);
double round(double x);
long lround(double x);
long long llround(double x);
double trunc(double x);
double fmod(double x, double y);
double remainder(double x, double y);
double remquo(double x, double y, int *quo);
double copysign(double x, double y);
double nan(const char *tagp);
double nextafter(double x, double y);
double fdim(double x, double y);
double fmax(double x, double y);
double fmin(double x, double y);
double fma(double x, double y, double z);

float acosf(float x);
float asinf(float x);
float atanf(float x);
float atan2f(float y, float x);
float cosf(float x);
float sinf(float x);
float tanf(float x);
float acoshf(float x);
float asinhf(float x);
float atanhf(float x);
float coshf(float x);
float sinhf(float x);
float tanhf(float x);
float expf(float x);
float exp2f(float x);
float expm1f(float x);
float frexpf(float x, int *exp);
int ilogbf(float x);
float ldexpf(float x, int exp);
float logf(float x);
float log10f(float x);
float log1pf(float x);
float log2f(float x);
float logbf(float x);
float modff(float x, float *iptr);
float scalbnf(float x, int n);
float scalblnf(float x, long n);
float cbrtf(float x);
float fabsf(float x);
float hypotf(float x, float y);
float powf(float x, float y);
float sqrtf(float x);
float erff(float x);
float erfcf(float x);
float lgammaf(float x);
float tgammaf(float x);
float ceilf(float x);
float floorf(float x);
float nearbyintf(float x);
float rintf(float x);
long lrintf(float x);
long long llrintf(float x);
float roundf(float x);
long lroundf(float x);
long long llroundf(float x);
float truncf(float x);
float fmodf(float x, float y);
float remainderf(float x, float y);
float remquof(float x, float y, int *quo);
float copysignf(float x, float y);
float nanf(const char *tagp);
float nextafterf(float x, float y);
float fdimf(float x, float y);
float fmaxf(float x, float y);
float fminf(float x, float y);
float fmaf(float x, float y, float z);

/* nexttoward takes its direction as a long double: a double or binary128, whose comparisons cstd
 * makes at the type's own width. (x86's 80-bit format has no counterpart in cstd.) */
#if __LDBL_MANT_DIG__ == 53 || __LDBL_MANT_DIG__ == 113
double nexttoward(double x, long double y);
float nexttowardf(float x, long double y);
#endif

/* The long double functions exist where long double is binary64, so each is its double counterpart
 * under a second name. Where long double is binary128 (AArch64 Linux, RISC-V) they are not offered
 * yet: quad-precision versions of the algorithms are still to be written. */
#if __LDBL_MANT_DIG__ == 53
long double acosl(long double x);
long double asinl(long double x);
long double atanl(long double x);
long double atan2l(long double y, long double x);
long double cosl(long double x);
long double sinl(long double x);
long double tanl(long double x);
long double acoshl(long double x);
long double asinhl(long double x);
long double atanhl(long double x);
long double coshl(long double x);
long double sinhl(long double x);
long double tanhl(long double x);
long double expl(long double x);
long double exp2l(long double x);
long double expm1l(long double x);
long double frexpl(long double x, int *exp);
int ilogbl(long double x);
long double ldexpl(long double x, int exp);
long double logl(long double x);
long double log10l(long double x);
long double log1pl(long double x);
long double log2l(long double x);
long double logbl(long double x);
long double modfl(long double x, long double *iptr);
long double scalbnl(long double x, int n);
long double scalblnl(long double x, long n);
long double cbrtl(long double x);
long double fabsl(long double x);
long double hypotl(long double x, long double y);
long double powl(long double x, long double y);
long double sqrtl(long double x);
long double erfl(long double x);
long double erfcl(long double x);
long double lgammal(long double x);
long double tgammal(long double x);
long double ceill(long double x);
long double floorl(long double x);
long double nearbyintl(long double x);
long double rintl(long double x);
long lrintl(long double x);
long long llrintl(long double x);
long double roundl(long double x);
long lroundl(long double x);
long long llroundl(long double x);
long double truncl(long double x);
long double fmodl(long double x, long double y);
long double remainderl(long double x, long double y);
long double remquol(long double x, long double y, int *quo);
long double copysignl(long double x, long double y);
long double nanl(const char *tagp);
long double nextafterl(long double x, long double y);
long double nexttowardl(long double x, long double y);
long double fdiml(long double x, long double y);
long double fmaxl(long double x, long double y);
long double fminl(long double x, long double y);
long double fmal(long double x, long double y, long double z);
#endif

#ifdef __cplusplus
}
#endif

#endif
