/* <string.h> from sh.sysl.cstd: the byte and C-string functions.
 * Compile with -nostdinc -I <cstd>/include beside clang's own freestanding headers. */
#ifndef CSTD_STRING_H
#define CSTD_STRING_H

#include <stddef.h>

#ifndef __CSTD_LOCALE_T
#define __CSTD_LOCALE_T
typedef struct __cstd_locale *locale_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t n);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strcpy(char *__restrict dst, const char *__restrict src);
char *strncpy(char *__restrict dst, const char *__restrict src, size_t n);
char *strcat(char *__restrict dst, const char *__restrict src);
char *strncat(char *__restrict dst, const char *__restrict src, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
char *strpbrk(const char *s, const char *accept);
char *strtok(char *__restrict s, const char *__restrict delim);
char *strtok_r(char *__restrict s, const char *__restrict delim, char **__restrict save);
char *strerror(int code);
int strerror_r(int code, char *buf, size_t size);
char *strerror_l(int code, locale_t locale);
char *strsignal(int sig);
char *stpcpy(char *__restrict dst, const char *__restrict src);
char *stpncpy(char *__restrict dst, const char *__restrict src, size_t n);
size_t strlcpy(char *__restrict dst, const char *__restrict src, size_t size);
size_t strlcat(char *__restrict dst, const char *__restrict src, size_t size);
char *strdup(const char *s);
char *strndup(const char *s, size_t size);
int strcoll(const char *a, const char *b);
int strcoll_l(const char *a, const char *b, locale_t locale);
size_t strxfrm(char *__restrict dst, const char *__restrict src, size_t n);
size_t strxfrm_l(char *__restrict dst, const char *__restrict src, size_t n, locale_t locale);

void *memmem(const void *haystack, size_t haystack_len, const void *needle, size_t needle_len);
void *memcpy(void *__restrict dst, const void *__restrict src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
void *memchr(const void *s, int c, size_t n);

#ifdef __cplusplus
}
#endif

#endif
