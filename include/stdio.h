/* <stdio.h> from sh.sysl.cstd: the printf family, minus floating point. There is no FILE yet, so
 * printf writes to descriptor 1 through write(), and dprintf to the descriptor it is given. See the
 * README for what each conversion prints and what is deliberately absent. */
#ifndef CSTD_STDIO_H
#define CSTD_STDIO_H

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char *restrict fmt, ...);
int dprintf(int fd, const char *restrict fmt, ...);
int sprintf(char *restrict buf, const char *restrict fmt, ...);
int snprintf(char *restrict buf, size_t n, const char *restrict fmt, ...);
int vprintf(const char *restrict fmt, va_list ap);
int vdprintf(int fd, const char *restrict fmt, va_list ap);
int vsprintf(char *restrict buf, const char *restrict fmt, va_list ap);
int vsnprintf(char *restrict buf, size_t n, const char *restrict fmt, va_list ap);

#ifdef __cplusplus
}
#endif

#endif
