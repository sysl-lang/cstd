/* What every program under tests/linux/ reports through: cstd's own `write` to standard output and
 * nothing else. A program states each expectation as `check(condition)` — the condition written from
 * POSIX's text or Linux's documented behaviour — and a failed one prints its line, its text and the
 * errno at that moment; a program with no failure prints `<name>: ok` at the end. The test that runs
 * it compares the whole output and the exit status. */
#ifndef CSTD_TESTS_LINUX_CHECK_H
#define CSTD_TESTS_LINUX_CHECK_H

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

static int failures;

static void say(const char *s) { write(1, s, strlen(s)); }

static void say_num(long long v) {
    char digits[24];
    int n = 0;
    unsigned long long u = v < 0 ? 0ULL - (unsigned long long)v : (unsigned long long)v;
    do {
        digits[n++] = (char)('0' + u % 10);
        u /= 10;
    } while (u != 0);
    if (v < 0) write(1, "-", 1);
    while (n > 0) write(1, &digits[--n], 1);
}

static void say_line(const char *s) {
    say(s);
    say("\n");
}

static void failed(int line, const char *text) {
    int e = errno;
    failures++;
    say("FAIL line ");
    say_num(line);
    say(": ");
    say(text);
    say(" (errno ");
    say_num(e);
    say(")\n");
}

#define check(cond) ((cond) ? (void)0 : failed(__LINE__, #cond))

/* A call that must fail with -1 and the named errno; errno is cleared first, so a stale value cannot
 * pass. */
#define check_fails(call, code) \
    do { \
        errno = 0; \
        long long answer_ = (long long)(call); \
        if (answer_ != -1 || errno != (code)) failed(__LINE__, #call " == -1 with " #code); \
    } while (0)

static int done(const char *name) {
    if (failures == 0) {
        say(name);
        say(": ok\n");
    }
    return failures == 0 ? 0 : 1;
}

#endif
