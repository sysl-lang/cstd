/* <time.h> from sh.sysl.cstd.
 *
 * The calendar (gmtime, timegm, mktime, localtime, strftime, ...) is cstd's own and holds on every
 * target. There is no time-zone database: local time is UTC.
 *
 * The clocks, the sleeps and the timers are Linux's system calls, defined by cstd's Linux half; on
 * any other bare-metal target each is a weak default answering -1 with errno ENOSYS (clock_nanosleep
 * answers ENOSYS itself), which a board replaces with a definition of its own. time, clock and
 * timespec_get read the clock through clock_gettime. */
#ifndef CSTD_TIME_H
#define CSTD_TIME_H

#include <stddef.h>
#include <sys/types.h>

#ifndef __CSTD_LOCALE_T
#define __CSTD_LOCALE_T
typedef struct __cstd_locale *locale_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef long clock_t;
typedef int timer_t;

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    long tm_gmtoff;
    const char *tm_zone;
};

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

struct itimerspec {
    struct timespec it_interval;
    struct timespec it_value;
};

/* As <signal.h> defines it; timer_create passes it to the kernel unread. */
struct sigevent;

#define CLOCKS_PER_SEC ((clock_t)1000000)
#define TIME_UTC 1
#define TIMER_ABSTIME 1

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID 3

clock_t clock(void);
double difftime(time_t a, time_t b);
struct tm *gmtime(const time_t *t);
struct tm *gmtime_r(const time_t *restrict t, struct tm *restrict tm);
struct tm *localtime(const time_t *t);
struct tm *localtime_r(const time_t *restrict t, struct tm *restrict tm);
time_t mktime(struct tm *tm);
time_t timegm(struct tm *tm);
size_t strftime(char *restrict s, size_t max, const char *restrict fmt, const struct tm *restrict tm);
size_t strftime_l(char *restrict s, size_t max, const char *restrict fmt, const struct tm *restrict tm,
                  locale_t locale);
time_t time(time_t *t);
int timespec_get(struct timespec *ts, int base);
void tzset(void);
extern char *tzname[2];

int clock_getres(clockid_t clock, struct timespec *res);
int clock_gettime(clockid_t clock, struct timespec *ts);
int clock_settime(clockid_t clock, const struct timespec *ts);
int clock_nanosleep(clockid_t clock, int flags, const struct timespec *req, struct timespec *rem);
int nanosleep(const struct timespec *req, struct timespec *rem);

int timer_create(clockid_t clock, struct sigevent *restrict event, timer_t *restrict timer);
int timer_delete(timer_t timer);
int timer_getoverrun(timer_t timer);
int timer_gettime(timer_t timer, struct itimerspec *value);
int timer_settime(timer_t timer, int flags, const struct itimerspec *restrict value,
                  struct itimerspec *restrict old);

#ifdef __cplusplus
}
#endif

#endif
