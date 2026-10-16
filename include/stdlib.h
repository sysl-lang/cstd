/* <stdlib.h> from sh.sysl.cstd: the number conversions, abs and div, qsort and bsearch everywhere.
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
long atol(const char *s);
double atof(const char *s);
long strtol(const char *__restrict s, char **__restrict endptr, int base);
long long strtoll(const char *__restrict s, char **__restrict endptr, int base);
unsigned long strtoul(const char *__restrict s, char **__restrict endptr, int base);
unsigned long long strtoull(const char *__restrict s, char **__restrict endptr, int base);
double strtod(const char *__restrict s, char **__restrict endptr);
float strtof(const char *__restrict s, char **__restrict endptr);
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
