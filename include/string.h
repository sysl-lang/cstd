/* <string.h> from sh.sysl.cstd: the byte and C-string functions.
 * Compile with -nostdinc -I <cstd>/include beside clang's own freestanding headers. */
#ifndef CSTD_STRING_H
#define CSTD_STRING_H

#include <stddef.h>

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

void *memcpy(void *__restrict dst, const void *__restrict src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
void *memchr(const void *s, int c, size_t n);

#ifdef __cplusplus
}
#endif

#endif
