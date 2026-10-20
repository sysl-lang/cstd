/* <stdio.h> from sh.sysl.cstd: the printf family, minus floating point. There is no FILE yet, so
 * printf writes to descriptor 1 through write(), and dprintf to the descriptor it is given; puts and
 * putchar write to descriptor 1 the same way. See the
 * README for what each conversion prints and what is deliberately absent.
 *
 * `__restrict` rather than `restrict`: the header is also read as C++ (clangd reads a .h that way),
 * where `restrict` is no keyword and every prototype fails to parse. */
#ifndef CSTD_STDIO_H
#define CSTD_STDIO_H

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EOF
#define EOF (-1)
#endif

int puts(const char *s);
int putchar(int c);
int printf(const char *__restrict fmt, ...);
int dprintf(int fd, const char *__restrict fmt, ...);
int sprintf(char *__restrict buf, const char *__restrict fmt, ...);
int snprintf(char *__restrict buf, size_t n, const char *__restrict fmt, ...);
int vprintf(const char *__restrict fmt, va_list ap);
int vdprintf(int fd, const char *__restrict fmt, va_list ap);
int vsprintf(char *__restrict buf, const char *__restrict fmt, va_list ap);
int vsnprintf(char *__restrict buf, size_t n, const char *__restrict fmt, va_list ap);

#ifdef __cplusplus
}
#endif

#endif
