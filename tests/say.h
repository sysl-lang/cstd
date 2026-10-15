/* The test prelude every case is compiled with: the small printers a case reports through.
 *
 * Nothing here comes from the host's libc but `write` — no printf, no strlen — so what a case prints
 * is what cstd answered and nothing else. Every printer ends its line with a newline.
 *
 * Built with -fno-builtin -O0, so clang neither folds a call to a cstd function nor turns a loop
 * here into one. */
#ifndef CSTD_TESTS_SAY_H
#define CSTD_TESTS_SAY_H

#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

long write(int fd, const void *buf, unsigned long n);

static void say_raw(const char *s, size_t n) { write(1, s, n); }

static void say_ch(char c) { say_raw(&c, 1); }

static void say_end(void) { say_ch('\n'); }

static void put_u(unsigned long long v) {
    char digits[24];
    int n = 0;
    do {
        digits[n++] = (char)('0' + v % 10);
        v /= 10;
    } while (v != 0);
    while (n > 0) say_ch(digits[--n]);
}

static void put_i(long long v) {
    if (v < 0) {
        say_ch('-');
        put_u(0ULL - (unsigned long long)v);
    } else {
        put_u((unsigned long long)v);
    }
}

/* A signed number, in decimal. */
static void say_int(long long v) { put_i(v); say_end(); }

/* An unsigned number, in decimal. */
static void say_uint(unsigned long long v) { put_u(v); say_end(); }

/* A C string as it stands, or NULL for a null pointer. */
static void say_str(const char *s) {
    if (s == NULL) {
        say_raw("NULL", 4);
    } else {
        while (*s != 0) say_ch(*s++);
    }
    say_end();
}

/* Exactly n bytes, NULs and all. A printable byte is itself; every other byte, and '[' so the
 * form reads one way, is its two hex digits in brackets: "ab[00][ff]". */
static void put_bytes(const void *p, size_t n) {
    const unsigned char *b = p;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = b[i];
        if (c >= 0x20 && c < 0x7f && c != '[') {
            say_ch((char)c);
        } else {
            say_ch('[');
            say_ch("0123456789abcdef"[c >> 4]);
            say_ch("0123456789abcdef"[c & 15]);
            say_ch(']');
        }
    }
}

static void say_bytes(const void *p, size_t n) { put_bytes(p, n); say_end(); }

/* One vsnprintf into a roomy buffer, on one line: the count it answered, then what it wrote between
 * bars, in say_bytes's form so a space at either edge shows: "5|   42|". */
static void say_f(const char *fmt, ...) {
    char b[1024];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(b, sizeof b, fmt, ap);
    va_end(ap);
    put_i(n);
    say_ch('|');
    if (n > 0) put_bytes(b, (size_t)n < sizeof b ? (size_t)n : sizeof b - 1);
    say_ch('|');
    say_end();
}

/* 1 when two pointers are the same address, 0 when not. */
static void say_ptr_eq(const void *a, const void *b) { say_int(a == b); }

/* Where p points, as an offset from base; NULL for a null pointer. */
static void say_off(const void *p, const void *base) {
    if (p == NULL) {
        say_str(NULL);
    } else {
        say_int((const char *)p - (const char *)base);
    }
}

/* The sign of a comparison's answer: -1, 0 or 1. */
static void say_sign(int v) { say_int(v < 0 ? -1 : v > 0); }

/* Which arguments in EOF, 0..255 a classifier says yes to, as ranges: "48-57,65-70". A yes that
 * is not exactly 1 is printed with a '!' after it, as this library promises 1. */
static void say_class(int (*f)(int)) {
    int first = 1;
    int c = -1;
    while (c <= 255) {
        int v = f(c);
        if (v == 0) {
            c++;
            continue;
        }
        int lo = c;
        while (c + 1 <= 255 && f(c + 1) == v) c++;
        if (!first) say_ch(',');
        first = 0;
        put_i(lo);
        if (c != lo) {
            say_ch('-');
            put_i(c);
        }
        if (v != 1) say_ch('!');
        c++;
    }
    say_end();
}

/* Which arguments in EOF, 0..255 a case mapping moves, as ranges, each followed by how far it
 * moves them: "65-90:+32". */
static void say_mapping(int (*f)(int)) {
    int first = 1;
    int c = -1;
    while (c <= 255) {
        int d = f(c) - c;
        if (d == 0) {
            c++;
            continue;
        }
        int lo = c;
        while (c + 1 <= 255 && f(c + 1) - (c + 1) == d) c++;
        if (!first) say_ch(',');
        first = 0;
        put_i(lo);
        if (c != lo) {
            say_ch('-');
            put_i(c);
        }
        say_ch(':');
        if (d > 0) say_ch('+');
        put_i(d);
        c++;
    }
    say_end();
}

/* One reading by a strto* function, on one line: the value, where endptr was left (as an offset
 * into the text), and errno, which is cleared first — "-42 3 0". */
static void say_strtol(const char *s, int base) {
    char *end = (char *)"unset";
    errno = 0;
    long v = strtol(s, &end, base);
    put_i(v); say_ch(' '); put_i(end - s); say_ch(' '); say_int(errno);
}

static void say_strtoll(const char *s, int base) {
    char *end = (char *)"unset";
    errno = 0;
    long long v = strtoll(s, &end, base);
    put_i(v); say_ch(' '); put_i(end - s); say_ch(' '); say_int(errno);
}

static void say_strtoul(const char *s, int base) {
    char *end = (char *)"unset";
    errno = 0;
    unsigned long v = strtoul(s, &end, base);
    put_u(v); say_ch(' '); put_i(end - s); say_ch(' '); say_int(errno);
}

static void say_strtoull(const char *s, int base) {
    char *end = (char *)"unset";
    errno = 0;
    unsigned long long v = strtoull(s, &end, base);
    put_u(v); say_ch(' '); put_i(end - s); say_ch(' '); say_int(errno);
}

/* `digits` hex digits of v, most significant first, without going through printf. */
static void put_hex(unsigned long long v, int digits) {
    for (int i = digits - 1; i >= 0; i--) say_ch("0123456789abcdef"[(v >> (4 * i)) & 15]);
}

/* strtod's answer as the double's bits in hex, how far endptr moved, and errno. */
static void say_strtod(const char *s) {
    char *end = (char *)"unset";
    errno = 0;
    double v = strtod(s, &end);
    unsigned long long bits;
    memcpy(&bits, &v, sizeof bits);
    put_hex(bits, 16); say_ch(' '); put_i(end - s); say_ch(' '); say_int(errno);
}

static void say_strtof(const char *s) {
    char *end = (char *)"unset";
    errno = 0;
    float v = strtof(s, &end);
    unsigned int bits;
    memcpy(&bits, &v, sizeof bits);
    put_hex(bits, 8); say_ch(' '); put_i(end - s); say_ch(' '); say_int(errno);
}

/* A double with exactly these bits. */
static double from_bits(unsigned long long bits) {
    double v;
    memcpy(&v, &bits, sizeof v);
    return v;
}

#endif
