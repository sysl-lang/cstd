/* <strings.h> from sh.sysl.cstd: case-insensitive comparison and bit search.
 * Only the C locale exists, so case folding is ASCII and the _l forms ignore their locale. */
#ifndef CSTD_STRINGS_H
#define CSTD_STRINGS_H

#include <stddef.h>

#ifndef __CSTD_LOCALE_T
#define __CSTD_LOCALE_T
typedef struct __cstd_locale *locale_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

int strcasecmp(const char *a, const char *b);
int strcasecmp_l(const char *a, const char *b, locale_t locale);
int strncasecmp(const char *a, const char *b, size_t n);
int strncasecmp_l(const char *a, const char *b, size_t n, locale_t locale);

/* POSIX marks these three [XSI]; they are offered because they are cheap and every libc has them. */
int ffs(int i);
int ffsl(long i);
int ffsll(long long i);

#ifdef __cplusplus
}
#endif

#endif
