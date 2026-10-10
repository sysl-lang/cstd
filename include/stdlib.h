/* <stdlib.h> from sh.sysl.cstd: the number conversions, abs and div, qsort and bsearch. Nothing
 * that allocates or exits is here yet; see the README for what is deliberately absent. */
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

#ifdef __cplusplus
}
#endif

#endif
