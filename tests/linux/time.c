/* The clocks, the sleeps and the timers (POSIX clock_getres, clock_nanosleep, nanosleep,
 * timer_create, timer_delete, timer_getoverrun, timer_settime). argv[1] is the host's time in seconds
 * since the Epoch, read just before the run: the VM's clock is kept to the host's, so CLOCK_REALTIME
 * must agree with it to within a minute. Every call but `clock_nanosleep` answers -1 with errno;
 * `clock_nanosleep` answers the error number itself and leaves errno alone. */
#include "check.h"

#include <time.h>

/* Linux's `struct sigevent` (64 bytes: the value, the signal, how to notify, then room) asking for
 * no notification at all; no cstd header defines the structure, and timer_create hands it to the
 * kernel unread. */
struct linux_sigevent {
    long value;
    int signo;
    int notify;
    int pad[12];
};
#define LINUX_SIGEV_NONE 1

static long long ns(const struct timespec *t) { return (long long)t->tv_sec * 1000000000LL + t->tv_nsec; }

static long long now_ns(clockid_t clock) {
    struct timespec t;
    clock_gettime(clock, &t);
    return ns(&t);
}

static int valid(const struct timespec *t) { return t->tv_sec >= 0 && t->tv_nsec >= 0 && t->tv_nsec < 1000000000L; }

static void clocks(long host) {
    struct timespec t, u;
    check(clock_gettime(CLOCK_REALTIME, &t) == 0 && valid(&t));
    check(t.tv_sec > host - 60 && t.tv_sec < host + 60);
    check(clock_gettime(CLOCK_MONOTONIC, &t) == 0 && valid(&t));
    check(clock_gettime(CLOCK_MONOTONIC, &u) == 0 && ns(&u) >= ns(&t));
    check(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t) == 0 && valid(&t));
    check(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t) == 0 && valid(&t));
    check_fails(clock_gettime(99, &t), EINVAL);
    check_fails(clock_gettime(CLOCK_REALTIME, (struct timespec *)8), EFAULT);

    /* High-resolution timers make each clock's resolution a nanosecond; at worst it is a tick. */
    struct timespec res;
    check(clock_getres(CLOCK_REALTIME, &res) == 0 && res.tv_sec == 0 && res.tv_nsec > 0 && res.tv_nsec <= 10000000);
    check(clock_getres(CLOCK_MONOTONIC, &res) == 0 && res.tv_sec == 0 && res.tv_nsec > 0 && res.tv_nsec <= 10000000);
    /* A null res is allowed, and the clock is still checked. */
    check(clock_getres(CLOCK_MONOTONIC, NULL) == 0);
    check_fails(clock_getres(99, &res), EINVAL);
    check_fails(clock_getres(99, NULL), EINVAL);

    /* CLOCK_MONOTONIC cannot be set; CLOCK_REALTIME can, but not by this user; a bad time is EINVAL. */
    check(clock_gettime(CLOCK_REALTIME, &t) == 0);
    check_fails(clock_settime(CLOCK_MONOTONIC, &t), EINVAL);
    check_fails(clock_settime(CLOCK_REALTIME, &t), EPERM);
    check_fails(clock_settime(99, &t), EINVAL);
}

static void sleeps(void) {
    struct timespec d = {0, 20000000}, rem = {7, 7};
    long long before = now_ns(CLOCK_MONOTONIC);
    check(nanosleep(&d, &rem) == 0);
    check(now_ns(CLOCK_MONOTONIC) - before >= 20000000);
    struct timespec zero = {0, 0};
    check(nanosleep(&zero, NULL) == 0);
    struct timespec bad = {0, 1000000000L};
    check_fails(nanosleep(&bad, NULL), EINVAL);
    struct timespec negative = {-1, 0};
    check_fails(nanosleep(&negative, NULL), EINVAL);
    struct timespec negative_ns = {0, -1};
    check_fails(nanosleep(&negative_ns, NULL), EINVAL);
    check_fails(nanosleep((const struct timespec *)8, NULL), EFAULT);

    /* clock_nanosleep: a relative sleep, an absolute one, a deadline already past. */
    struct timespec ten = {0, 10000000};
    before = now_ns(CLOCK_MONOTONIC);
    errno = 0;
    check(clock_nanosleep(CLOCK_MONOTONIC, 0, &ten, NULL) == 0);
    check(now_ns(CLOCK_MONOTONIC) - before >= 10000000);
    struct timespec deadline;
    clock_gettime(CLOCK_MONOTONIC, &deadline);
    long long target = ns(&deadline) + 15000000;
    deadline.tv_sec = target / 1000000000LL;
    deadline.tv_nsec = target % 1000000000LL;
    check(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL) == 0);
    check(now_ns(CLOCK_MONOTONIC) >= target);
    struct timespec past = {0, 1};
    before = now_ns(CLOCK_MONOTONIC);
    check(clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &past, NULL) == 0);
    check(now_ns(CLOCK_MONOTONIC) - before < 1000000000LL);
    /* Errors come back as the answer, and errno is untouched. */
    errno = 0;
    check(clock_nanosleep(CLOCK_MONOTONIC, 0, &bad, NULL) == EINVAL);
    check(clock_nanosleep(99, 0, &ten, NULL) == EINVAL);
    /* The calling thread's CPU-time clock is refused (POSIX: EINVAL). */
    check(clock_nanosleep(CLOCK_THREAD_CPUTIME_ID, 0, &ten, NULL) == EINVAL);
    check(clock_nanosleep(CLOCK_MONOTONIC, 0, (const struct timespec *)8, NULL) == EFAULT);
    check(errno == 0);
}

static void timers(void) {
    struct linux_sigevent none = {0, 0, LINUX_SIGEV_NONE, {0}};
    timer_t t = -1;
    check(timer_create(CLOCK_MONOTONIC, (struct sigevent *)&none, &t) == 0);
    check(t >= 0);
    struct itimerspec value;
    memset(&value, 0x55, sizeof value);
    /* A new timer is disarmed: all zero. */
    check(timer_gettime(t, &value) == 0);
    check(value.it_value.tv_sec == 0 && value.it_value.tv_nsec == 0);
    check(value.it_interval.tv_sec == 0 && value.it_interval.tv_nsec == 0);
    check(timer_getoverrun(t) == 0);

    /* Armed for ten seconds repeating every second; the old setting comes back disarmed. */
    struct itimerspec arm = {{1, 0}, {10, 0}}, old;
    memset(&old, 0x55, sizeof old);
    check(timer_settime(t, 0, &arm, &old) == 0);
    check(old.it_value.tv_sec == 0 && old.it_value.tv_nsec == 0 && old.it_interval.tv_sec == 0);
    check(timer_gettime(t, &value) == 0);
    check(value.it_interval.tv_sec == 1 && value.it_interval.tv_nsec == 0);
    check(value.it_value.tv_sec == 9 || (value.it_value.tv_sec == 10 && value.it_value.tv_nsec == 0));
    /* Re-arming answers the setting it replaced. */
    struct itimerspec once = {{0, 0}, {0, 1000000}};
    check(timer_settime(t, 0, &once, &old) == 0);
    check(old.it_interval.tv_sec == 1 && (old.it_value.tv_sec == 9 || old.it_value.tv_sec == 10));
    /* A one-shot of a millisecond has expired twenty milliseconds later, and is disarmed. */
    struct timespec wait = {0, 20000000};
    nanosleep(&wait, NULL);
    check(timer_gettime(t, &value) == 0);
    check(value.it_value.tv_sec == 0 && value.it_value.tv_nsec == 0);
    /* An absolute expiry already past fires at once. */
    struct itimerspec past = {{0, 0}, {0, 1}};
    check(timer_settime(t, TIMER_ABSTIME, &past, NULL) == 0);
    check(timer_gettime(t, &value) == 0 && value.it_value.tv_sec == 0 && value.it_value.tv_nsec == 0);
    /* A nanosecond count out of range is EINVAL. */
    struct itimerspec bad = {{0, 0}, {0, 1000000000L}};
    check_fails(timer_settime(t, 0, &bad, NULL), EINVAL);
    check(timer_delete(t) == 0);

    /* A deleted timer, and an id never made, are EINVAL to every call. */
    check_fails(timer_delete(t), EINVAL);
    check_fails(timer_gettime(t, &value), EINVAL);
    check_fails(timer_settime(t, 0, &arm, NULL), EINVAL);
    check_fails(timer_getoverrun(t), EINVAL);
    check_fails(timer_getoverrun(12345), EINVAL);

    /* No sigevent is SIGALRM to the process; created and deleted unarmed, it never fires. */
    timer_t plain = -1;
    check(timer_create(CLOCK_REALTIME, NULL, &plain) == 0 && plain >= 0);
    check(timer_delete(plain) == 0);
    check_fails(timer_create(99, NULL, &plain), EINVAL);
    check_fails(timer_create(CLOCK_MONOTONIC, NULL, (timer_t *)8), EFAULT);
}

int main(int argc, char **argv) {
    if (argc != 2) return 99;
    long host = 0;
    for (const char *p = argv[1]; *p; p++) host = host * 10 + (*p - '0');
    clocks(host);
    sleeps();
    timers();
    return done("time");
}
