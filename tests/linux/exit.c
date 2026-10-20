/* How a process ends, one way per run, chosen by argv[1]. `exit` and a return from `main` call the
 * `atexit` handlers in the reverse of their registration and then end the process with the status
 * (C 7.24.4.4); `_exit` and `_Exit` end it with no handler run (POSIX _exit); `abort` ends it by
 * SIGABRT with no handler run (C 7.24.4.1), blocked or ignored as the signal may be. The parent sees only the status's low eight bits (POSIX
 * wait: "the low-order 8 bits of the status argument"). At least 32 handlers can be registered
 * (C 7.24.4.2); cstd's table holds exactly 32, so the 33rd answers nonzero. */
#include "check.h"

#include <stdlib.h>

/* `sigprocmask` under a name of this file's own, as Linux's aarch64 ABI takes it: one 64-bit word,
 * bit n-1 for signal n; SIG_BLOCK is 0. */
int sigprocmask_abi(int how, const unsigned long *set, unsigned long *old) __asm__("sigprocmask");

static void first(void) { say_line("handler first"); }
static void second(void) { say_line("handler second"); }
static void third(void) { say_line("handler third"); }

static void deep(int status) { exit(status); }

static int handled;
static void count(void) { handled++; }
static void report(void) {
    say("handlers run before this one: ");
    say_num(handled);
    say("\n");
}

int main(int argc, char **argv) {
    if (argc != 2) return 99;
    const char *how = argv[1];

    if (strcmp(how, "table") == 0) {
        check(atexit(report) == 0);
        for (int i = 0; i < 31; i++) check(atexit(count) == 0);
        check(atexit(count) != 0);
        done("table");
        return 0;
    }

    check(atexit(first) == 0);
    check(atexit(second) == 0);
    check(atexit(third) == 0);
    if (done(how) != 0) return 1;

    if (strcmp(how, "exit") == 0) deep(3);
    if (strcmp(how, "return") == 0) return 5;
    if (strcmp(how, "wide") == 0) exit(256 + 9);
    if (strcmp(how, "_exit") == 0) _exit(4);
    if (strcmp(how, "_Exit") == 0) _Exit(6);
    if (strcmp(how, "abort") == 0) abort();
    /* abort overrides SIGABRT being blocked (POSIX abort), and being ignored, which the test's shell
     * arranges with `trap '' ABRT` before running this. */
    if (strcmp(how, "abort-blocked") == 0) {
        unsigned long abrt = 1UL << (6 - 1);
        sigprocmask_abi(0, &abrt, NULL);
        abort();
    }
    if (strcmp(how, "abort-ignored") == 0) abort();
    return 98;
}
