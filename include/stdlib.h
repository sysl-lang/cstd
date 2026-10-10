/* <stdlib.h> from sh.sysl.cstd: the integer conversions and abs. Nothing that allocates, exits
 * or sorts is here yet; see the README for what is deliberately absent. */
#ifndef CSTD_STDLIB_H
#define CSTD_STDLIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int atoi(const char *s);
long atol(const char *s);
long strtol(const char *restrict s, char **restrict endptr, int base);
long long strtoll(const char *restrict s, char **restrict endptr, int base);
unsigned long strtoul(const char *restrict s, char **restrict endptr, int base);
unsigned long long strtoull(const char *restrict s, char **restrict endptr, int base);
int abs(int n);
long labs(long n);

#ifdef __cplusplus
}
#endif

#endif
