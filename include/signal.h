/* <signal.h> from sh.sysl.cstd: kill and the signal numbers, Linux's. Defined by cstd's Linux
 * half; on any other bare-metal target `kill` is a weak default answering -1 with errno ENOSYS,
 * which a board replaces with a definition of its own. There is no sigaction yet. */
#ifndef CSTD_SIGNAL_H
#define CSTD_SIGNAL_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGCHLD 17

int kill(pid_t pid, int sig);

#ifdef __cplusplus
}
#endif

#endif
