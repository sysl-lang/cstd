/* cstd's `time`, `clock` and `timespec_get` on a bare-metal machine under QEMU, over a board's own
 * `clock_gettime`, defined below: strong, so it replaces cstd's weak ENOSYS default. The clock is
 * fixed, so every answer is exact. The calendar runs here too, where `time_t` is a 32-bit `long`.
 * Everything prints through Arm semihosting's SYS_WRITE0; the program ends with SYS_EXIT_EXTENDED,
 * its code the number of failed checks, and the last line is "clock: ok" only when every one held. */
#include <errno.h>
#include <time.h>

static void say(const char *s) {
    register int r0 __asm__("r0") = 0x04;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

static void quit(int code) {
    unsigned args[2] = {0x20026, (unsigned)code};
    register int r0 __asm__("r0") = 0x20;
    register unsigned *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;) {
    }
}

int clock_gettime(clockid_t clock, struct timespec *ts) {
    if (clock == CLOCK_REALTIME) {
        ts->tv_sec = 1709647629;
        ts->tv_nsec = 250000000;
        return 0;
    }
    if (clock == CLOCK_PROCESS_CPUTIME_ID) {
        ts->tv_sec = 2;
        ts->tv_nsec = 345678901;
        return 0;
    }
    errno = EINVAL;
    return -1;
}

static int failures;

static void check(const char *what, int held) {
    if (!held) {
        failures++;
        say("FAIL ");
        say(what);
        say("\n");
    }
}

static int same(const char *a, const char *b) {
    while (*a && *a == *b) a++, b++;
    return *a == *b;
}

int main(void) {
#if defined(__ARM_FP)
    *(volatile unsigned *)0xE000ED88 |= 0xFu << 20;
    __asm__ volatile("dsb\n isb" ::: "memory");
#endif
    check("time_t is a 32-bit long here", sizeof(time_t) == 4);

    time_t t = 0;
    check("time answers the clock's seconds", time(0) == 1709647629);
    check("time stores them too", time(&t) == 1709647629 && t == 1709647629);

    struct timespec ts = {0, 0};
    check("timespec_get answers TIME_UTC", timespec_get(&ts, TIME_UTC) == TIME_UTC);
    check("timespec_get fills the time", ts.tv_sec == 1709647629 && ts.tv_nsec == 250000000);
    check("timespec_get refuses another base", timespec_get(&ts, 2) == 0);

    check("clock is the processor time in microseconds", clock() == 2345678);
    check("CLOCKS_PER_SEC is a million", CLOCKS_PER_SEC == 1000000);

    char buf[64];
    struct tm tm;
    check("strftime of time's answer", strftime(buf, sizeof buf, "%c %z %Z", gmtime(&t)) == 34 &&
                                           same(buf, "Tue Mar  5 14:07:09 2024 +0000 UTC"));

    t = 2147483647;
    check("the last second a 32-bit time_t holds", gmtime_r(&t, &tm) != 0 &&
          strftime(buf, sizeof buf, "%F %T %a %j", &tm) && same(buf, "2038-01-19 03:14:07 Tue 019"));
    t = -2147483647 - 1;
    check("the first", gmtime_r(&t, &tm) != 0 &&
          strftime(buf, sizeof buf, "%F %T %a %j", &tm) && same(buf, "1901-12-13 20:45:52 Fri 347"));

    tm.tm_year = 138, tm.tm_mon = 0, tm.tm_mday = 19, tm.tm_hour = 3, tm.tm_min = 14, tm.tm_sec = 7;
    check("timegm reaches the last second", timegm(&tm) == 2147483647);
    tm.tm_sec = 8;
    errno = 0;
    check("timegm one second past it is EOVERFLOW", timegm(&tm) == -1 && errno == EOVERFLOW && tm.tm_sec == 8);

    check("difftime in double", difftime(2147483647, -2147483647 - 1) == 4294967295.0);

    if (failures == 0) say("clock: ok\n");
    quit(failures);
    return 0;
}
