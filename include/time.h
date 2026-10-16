/* <time.h> from sh.sysl.cstd: the clocks and the sleep, as Linux's system calls. Defined by cstd's
 * Linux half; on any other bare-metal target each is a weak default answering -1 with errno ENOSYS,
 * which a board replaces with a definition of its own. */
#ifndef CSTD_TIME_H
#define CSTD_TIME_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1

int nanosleep(const struct timespec *req, struct timespec *rem);
int clock_gettime(clockid_t clock, struct timespec *ts);

#ifdef __cplusplus
}
#endif

#endif
