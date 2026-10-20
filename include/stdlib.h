/* <stdlib.h> from sh.sysl.cstd: the number conversions, abs and div, qsort and bsearch, rand and
 * srand, and the C locale's multibyte conversions everywhere.
 * The allocator and the exits are cstd's Linux half's (linux/) where a C program for keel or a
 * static Linux program links it; on any other bare-metal target they are weak defaults a board
 * replaces: malloc and its family over sbrk (ENOMEM until the board defines sbrk), exit running the
 * atexit handlers and then _Exit, and _Exit, _exit and abort trapping. */
#ifndef CSTD_STDLIB_H
#define CSTD_STDLIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int atoi(const char *s);
char *getenv(const char *name);
long atol(const char *s);
long long atoll(const char *s);
double atof(const char *s);
long strtol(const char *__restrict s, char **__restrict endptr, int base);
long long strtoll(const char *__restrict s, char **__restrict endptr, int base);
unsigned long strtoul(const char *__restrict s, char **__restrict endptr, int base);
unsigned long long strtoull(const char *__restrict s, char **__restrict endptr, int base);
double strtod(const char *__restrict s, char **__restrict endptr);
float strtof(const char *__restrict s, char **__restrict endptr);
/* strtold exists where long double is binary64, as strtod under a second name; where long double is
 * binary128 (AArch64 Linux, RISC-V) it is not offered, as <math.h>'s l functions are not. */
#if __LDBL_MANT_DIG__ == 53
long double strtold(const char *__restrict s, char **__restrict endptr);
#endif
int abs(int n);
long labs(long n);
long long llabs(long long n);

typedef struct { int quot; int rem; } div_t;
typedef struct { long quot; long rem; } ldiv_t;
typedef struct { long long quot; long long rem; } lldiv_t;

div_t div(int n, int d);
ldiv_t ldiv(long n, long d);
lldiv_t lldiv(long long n, long long d);

void qsort(void *base, size_t n, size_t size, int (*cmp)(const void *, const void *));
void *bsearch(const void *key, const void *base, size_t n, size_t size,
              int (*cmp)(const void *, const void *));

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

/* rand is musl's 64-bit linear congruential generator, answering the state's top 31 bits. */
#define RAND_MAX 0x7fffffff
int rand(void);
void srand(unsigned seed);

/* The C locale's multibyte encoding: one byte per character, 0x00 to 0x7F, no shift state. */
#define MB_CUR_MAX ((size_t)1)
int mblen(const char *s, size_t n);
int mbtowc(wchar_t *__restrict pwc, const char *__restrict s, size_t n);
int wctomb(char *s, wchar_t wc);
size_t mbstowcs(wchar_t *__restrict pwcs, const char *__restrict s, size_t n);
size_t wcstombs(char *__restrict s, const wchar_t *__restrict pwcs, size_t n);

void *malloc(size_t n);
void *calloc(size_t count, size_t size);
void *realloc(void *p, size_t n);
void free(void *p);
_Noreturn void exit(int status);
_Noreturn void _Exit(int status);
_Noreturn void abort(void);
int atexit(void (*fn)(void));

#ifdef __cplusplus
}
#endif

#endif
