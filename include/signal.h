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

/* A signal set is one 64-bit word, signal n in bit n - 1, valid for signals 1 to 64. Linux's
 * rt_sigprocmask takes a mask of exactly 64 bits (its size argument is 8), and the exported
 * sigprocmask reads and writes the set through a plain pointer, so the struct is that one word.
 * `unsigned long long`, not `unsigned long`, keeps it 64 bits on the 32-bit targets as well. */
typedef struct {
    unsigned long long __bits[1];
} sigset_t;

/* Linux's values for the `how` of sigprocmask. */
#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

int kill(pid_t pid, int sig);

/* Defined by cstd's Linux half; a weak default elsewhere answers -1 with errno ENOSYS. Either set
 * may be null. */
int sigprocmask(int how, const sigset_t *__restrict set, sigset_t *__restrict oset);

/* Pure bit operations. sigaddset, sigdelset and sigismember answer -1 with errno EINVAL for a
 * signal number outside 1 to 64. */
int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int sig);
int sigdelset(sigset_t *set, int sig);
int sigismember(const sigset_t *set, int sig);

#ifdef __cplusplus
}
#endif

#endif
