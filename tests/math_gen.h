/* The input side of the <math.h> oracle tests: a fixed-seed generator of arguments, and printers
 * that call a cstd function on them and write one line per call -- the arguments and the result as
 * their bits in hex, then errno -- for the MPFR oracle in math_oracle_tests.sysl to judge.
 *
 * Like say.h it reaches the host for nothing but `write`. Output is gathered in a buffer, since a
 * case prints tens of thousands of lines. The generators use cstd's own arithmetic only to place
 * inputs, never to decide what an answer should be. */
#ifndef CSTD_TESTS_MATH_GEN_H
#define CSTD_TESTS_MATH_GEN_H

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

long write(int fd, const void *buf, unsigned long n);

static char gen_out[1 << 16];
static unsigned long gen_len;

static void gen_flush(void) {
    write(1, gen_out, gen_len);
    gen_len = 0;
}

static void gen_ch(char c) {
    if (gen_len == sizeof gen_out) gen_flush();
    gen_out[gen_len++] = c;
}

static void gen_hex(unsigned long long v, int digits) {
    for (int i = digits - 1; i >= 0; i--) gen_ch("0123456789abcdef"[(v >> (4 * i)) & 15]);
    gen_ch(' ');
}

static void gen_dec(long long v) {
    char d[24];
    int n = 0;
    unsigned long long u = v < 0 ? 0ULL - (unsigned long long)v : (unsigned long long)v;
    do { d[n++] = (char)('0' + u % 10); u /= 10; } while (u != 0);
    if (v < 0) gen_ch('-');
    while (n > 0) gen_ch(d[--n]);
    gen_ch(' ');
}

static void gen_end(void) {
    gen_dec(errno);
    gen_out[gen_len - 1] = '\n';
}

static uint64_t dbits(double x) { uint64_t u; memcpy(&u, &x, 8); return u; }
static double dfrom(uint64_t u) { double x; memcpy(&x, &u, 8); return x; }
static uint32_t fbits(float x) { uint32_t u; memcpy(&u, &x, 4); return u; }
static float ffrom(uint32_t u) { float x; memcpy(&x, &u, 4); return x; }

static void gd(double x) { gen_hex(dbits(x), 16); }
static void gf(float x) { gen_hex(fbits(x), 8); }

/* xorshift64*, from a fixed seed: every run sees the same inputs. */
static uint64_t gen_state = 0x9e3779b97f4a7c15ULL;
static uint64_t rbits(void) {
    gen_state ^= gen_state >> 12;
    gen_state ^= gen_state << 25;
    gen_state ^= gen_state >> 27;
    return gen_state * 0x2545f4914f6cdd1dULL;
}

/* Uniform in [0, 1). */
static double u01(void) { return (double)(rbits() >> 11) * (1.0 / 9007199254740992.0); }

/* Any bit pattern at all: every binade, both signs, subnormals, infinities and NaNs alike. */
static double any_d(void) { return dfrom(rbits()); }
static float any_f(void) { return ffrom((uint32_t)rbits()); }

/* Uniform in [lo, hi). */
static double uni_d(double lo, double hi) { return lo + (hi - lo) * u01(); }
static float uni_f(float lo, float hi) { return (float)uni_d(lo, hi); }

/* A positive value with a binary exponent uniform in [lo, hi] and a random significand. */
static double mag_d(int lo, int hi) {
    int e = lo + (int)(rbits() % (uint64_t)(hi - lo + 1));
    uint64_t m = rbits() & 0x000fffffffffffffULL;
    if (e < -1022) return dfrom(m >> (-1022 - e)) ;
    return dfrom(((uint64_t)(e + 1023) << 52) | m);
}
static float mag_f(int lo, int hi) {
    int e = lo + (int)(rbits() % (uint64_t)(hi - lo + 1));
    uint32_t m = (uint32_t)rbits() & 0x7fffff;
    if (e < -126) return ffrom(m >> (-126 - e));
    return ffrom(((uint32_t)(e + 127) << 23) | m);
}

/* x with a random sign. */
static double sgn_d(double x) { return rbits() & 1 ? -x : x; }
static float sgn_f(float x) { return rbits() & 1 ? -x : x; }

/* x moved up to k units in the last place either way, through its bits. */
static double near_d(double x, int k) {
    long long step = (long long)(rbits() % (uint64_t)(2 * k + 1)) - k;
    return dfrom(dbits(x) + (uint64_t)step);
}
static float near_f(float x, int k) {
    long long step = (long long)(rbits() % (uint64_t)(2 * k + 1)) - k;
    return ffrom(fbits(x) + (uint32_t)step);
}

/* The values every function is tried at: zeros, infinities, NaNs, the ends of the subnormal and
 * normal ranges, one and its neighbours, and small integers and halves. */
static const uint64_t dspecial[] = {
    0x0000000000000000ULL, 0x8000000000000000ULL, 0x7ff0000000000000ULL, 0xfff0000000000000ULL,
    0x7ff8000000000000ULL, 0xfff8000000000000ULL, 0x0000000000000001ULL, 0x8000000000000001ULL,
    0x000fffffffffffffULL, 0x800fffffffffffffULL, 0x0010000000000000ULL, 0x8010000000000000ULL,
    0x7fefffffffffffffULL, 0xffefffffffffffffULL, 0x3ff0000000000000ULL, 0xbff0000000000000ULL,
    0x3ff0000000000001ULL, 0x3fefffffffffffffULL, 0xbff0000000000001ULL, 0xbfefffffffffffffULL,
    0x3fe0000000000000ULL, 0xbfe0000000000000ULL, 0x4000000000000000ULL, 0xc000000000000000ULL,
    0x4008000000000000ULL, 0xc008000000000000ULL, 0x3ff8000000000000ULL, 0xbff8000000000000ULL,
    0x4004000000000000ULL, 0x4024000000000000ULL, 0x3e50000000000000ULL, 0x4330000000000000ULL,
    0x4330000000000001ULL, 0x433fffffffffffffULL, 0x400921fb54442d18ULL, 0x3ff921fb54442d18ULL,
};
#define DSPECIALS (int)(sizeof dspecial / sizeof dspecial[0])

static const uint32_t fspecial[] = {
    0x00000000, 0x80000000, 0x7f800000, 0xff800000, 0x7fc00000, 0xffc00000, 0x00000001, 0x80000001,
    0x007fffff, 0x807fffff, 0x00800000, 0x80800000, 0x7f7fffff, 0xff7fffff, 0x3f800000, 0xbf800000,
    0x3f800001, 0x3f7fffff, 0xbf800001, 0xbf7fffff, 0x3f000000, 0xbf000000, 0x40000000, 0xc0000000,
    0x40400000, 0xc0400000, 0x3fc00000, 0xbfc00000, 0x40200000, 0x41200000, 0x32800000, 0x4b000000,
    0x4b000001, 0x4b7fffff, 0x40490fdb, 0x3fc90fdb,
};
#define FSPECIALS (int)(sizeof fspecial / sizeof fspecial[0])

/* One call each, and its line. */
#define D1(f, x) do { double a_ = (x); errno = 0; double r_ = f(a_); \
    gd(a_); gd(r_); gen_end(); } while (0)
#define F1(f, x) do { float a_ = (x); errno = 0; float r_ = f(a_); \
    gf(a_); gf(r_); gen_end(); } while (0)
#define D2(f, x, y) do { double a_ = (x), b_ = (y); errno = 0; double r_ = f(a_, b_); \
    gd(a_); gd(b_); gd(r_); gen_end(); } while (0)
#define F2(f, x, y) do { float a_ = (x), b_ = (y); errno = 0; float r_ = f(a_, b_); \
    gf(a_); gf(b_); gf(r_); gen_end(); } while (0)
#define D3(f, x, y, z) do { double a_ = (x), b_ = (y), c_ = (z); errno = 0; double r_ = f(a_, b_, c_); \
    gd(a_); gd(b_); gd(c_); gd(r_); gen_end(); } while (0)
#define F3(f, x, y, z) do { float a_ = (x), b_ = (y), c_ = (z); errno = 0; float r_ = f(a_, b_, c_); \
    gf(a_); gf(b_); gf(c_); gf(r_); gen_end(); } while (0)
#define DI(f, x, n) do { double a_ = (x); long n_ = (n); errno = 0; double r_ = f(a_, n_); \
    gd(a_); gen_dec(n_); gd(r_); gen_end(); } while (0)
#define FI(f, x, n) do { float a_ = (x); long n_ = (n); errno = 0; float r_ = f(a_, n_); \
    gf(a_); gen_dec(n_); gf(r_); gen_end(); } while (0)
#define DFREXP(f, x) do { double a_ = (x); int n_ = 12345; errno = 0; double r_ = f(a_, &n_); \
    gd(a_); gd(r_); gen_dec(n_); gen_end(); } while (0)
#define FFREXP(f, x) do { float a_ = (x); int n_ = 12345; errno = 0; float r_ = f(a_, &n_); \
    gf(a_); gf(r_); gen_dec(n_); gen_end(); } while (0)
#define DMODF(f, x) do { double a_ = (x), i_ = 12345; errno = 0; double r_ = f(a_, &i_); \
    gd(a_); gd(r_); gd(i_); gen_end(); } while (0)
#define FMODF(f, x) do { float a_ = (x), i_ = 12345; errno = 0; float r_ = f(a_, &i_); \
    gf(a_); gf(r_); gf(i_); gen_end(); } while (0)
#define DREMQUO(f, x, y) do { double a_ = (x), b_ = (y); int q_ = 12345; errno = 0; \
    double r_ = f(a_, b_, &q_); gd(a_); gd(b_); gd(r_); gen_dec(q_); gen_end(); } while (0)
#define FREMQUO(f, x, y) do { float a_ = (x), b_ = (y); int q_ = 12345; errno = 0; \
    float r_ = f(a_, b_, &q_); gf(a_); gf(b_); gf(r_); gen_dec(q_); gen_end(); } while (0)
#define DINT(f, x) do { double a_ = (x); errno = 0; long long n_ = f(a_); \
    gd(a_); gen_dec(n_); gen_end(); } while (0)
#define FINT(f, x) do { float a_ = (x); errno = 0; long long n_ = f(a_); \
    gf(a_); gen_dec(n_); gen_end(); } while (0)

#endif
