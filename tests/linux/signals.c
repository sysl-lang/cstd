/* `getpid`, `getppid`, `kill` and `sigprocmask`, one way per run, chosen by argv[1]. The process ids
 * are checked against the kernel's own account of the process in /proc/self/stat (proc(5): field 1 is
 * the pid, field 4 the parent's). `kill` with signal 0 checks a process exists; an unknown signal is
 * EINVAL, a process that does not exist ESRCH, one this user may not signal EPERM (POSIX kill).
 * `sigprocmask` (POSIX pthread_sigmask) answers the previous mask, holds a blocked signal pending
 * rather than acting on it, and is EINVAL for an unknown `how`. The last two runs end the process by
 * a signal, which the shell reports as 128 plus its number. */
#include "check.h"

#include <fcntl.h>
#include <signal.h>

/* `sigprocmask` reached under a name of this file's own, as Linux's aarch64 ABI takes it, so the
 * declaration stands whatever <signal.h> says of it: the kernel's signal set is one 64-bit word, bit
 * n-1 for signal n, and SIG_BLOCK, SIG_UNBLOCK and SIG_SETMASK are 0, 1 and 2. */
int sigprocmask_abi(int how, const unsigned long *set, unsigned long *old) __asm__("sigprocmask");
#define sigprocmask sigprocmask_abi
#define MASK_BLOCK 0
#define MASK_UNBLOCK 1
#define MASK_SETMASK 2
#define BIT(sig) (1UL << ((sig) - 1))

/* The n-th space-separated field of /proc/self/stat, read with cstd's own calls; field 2, the
 * command name in parentheses, holds no space for these programs. */
static long stat_field(int n) {
    char text[1024];
    int fd = open("/proc/self/stat", O_RDONLY);
    if (fd < 0) return -1;
    long got = read(fd, text, sizeof text - 1);
    close(fd);
    if (got <= 0) return -1;
    text[got] = 0;
    const char *p = text;
    for (int field = 1; field < n; field++) {
        while (*p && *p != ' ') p++;
        if (*p) p++;
    }
    long v = 0;
    while (*p >= '0' && *p <= '9') v = v * 10 + (*p++ - '0');
    return v;
}

static void ids(void) {
    pid_t me = getpid();
    check(me > 1);
    check(getpid() == me);
    check(me == stat_field(1));
    pid_t parent = getppid();
    check(parent > 0 && parent != me);
    check(parent == stat_field(4));
}

static void signalling(void) {
    pid_t me = getpid();
    check(kill(me, 0) == 0);
    check(kill(getppid(), 0) == 0);
    check_fails(kill(me, 65), EINVAL);
    check_fails(kill(me, -1), EINVAL);
    /* pid_max is at most 2^22 on Linux, so this pid is never in use. */
    check_fails(kill(0x7ffffff0, 0), ESRCH);
    check_fails(kill(0x7ffffff0, SIGTERM), ESRCH);
    /* init belongs to root and this run does not. */
    check_fails(kill(1, SIGTERM), EPERM);
}

static void masking(void) {
    unsigned long old = 12345, now = 0;
    /* Nothing is blocked at the start of a program run from a shell. */
    check(sigprocmask(MASK_BLOCK, NULL, &old) == 0 && old == 0);
    unsigned long usr = BIT(SIGUSR1) | BIT(SIGUSR2);
    check(sigprocmask(MASK_BLOCK, &usr, &old) == 0 && old == 0);
    check(sigprocmask(MASK_BLOCK, NULL, &now) == 0 && now == usr);
    unsigned long one = BIT(SIGUSR2);
    check(sigprocmask(MASK_UNBLOCK, &one, &old) == 0 && old == usr);
    check(sigprocmask(MASK_SETMASK, NULL, &now) == 0 && now == BIT(SIGUSR1));
    /* SIGKILL cannot be blocked; asking is not an error, and it is left out (POSIX). */
    unsigned long kill_too = BIT(SIGKILL) | BIT(SIGUSR1);
    check(sigprocmask(MASK_SETMASK, &kill_too, NULL) == 0);
    check(sigprocmask(MASK_BLOCK, NULL, &now) == 0 && now == BIT(SIGUSR1));
    check_fails(sigprocmask(7, &usr, NULL), EINVAL);
    check_fails(sigprocmask(MASK_BLOCK, (const unsigned long *)8, NULL), EFAULT);

    /* A blocked signal sent to the process stays pending; the process carries on. */
    check(kill(getpid(), SIGUSR1) == 0);
    say_line("survived a blocked SIGUSR1");

    /* With SIGPIPE blocked, writing to a pipe nobody reads is EPIPE rather than death. */
    unsigned long pipe_bit = BIT(SIGPIPE);
    check(sigprocmask(MASK_BLOCK, &pipe_bit, NULL) == 0);
    int fds[2];
    check(pipe(fds) == 0);
    check(close(fds[0]) == 0);
    check_fails(write(fds[1], "x", 1), EPIPE);
    check(close(fds[1]) == 0);
}

int main(int argc, char **argv) {
    if (argc != 2) return 99;
    const char *how = argv[1];
    if (strcmp(how, "ids") == 0) {
        ids();
        return done("ids");
    }
    if (strcmp(how, "kill") == 0) {
        signalling();
        return done("kill");
    }
    if (strcmp(how, "mask") == 0) {
        masking();
        return done("mask");
    }
    if (strcmp(how, "die") == 0) {
        /* An unblocked SIGTERM to oneself ends the process before kill returns to anything after. */
        done("die");
        kill(getpid(), SIGTERM);
        say_line("still alive after SIGTERM");
        return 1;
    }
    if (strcmp(how, "unblock") == 0) {
        /* A pending signal is delivered once it is unblocked (POSIX: before sigprocmask returns). */
        unsigned long usr = BIT(SIGUSR1);
        check(sigprocmask(MASK_BLOCK, &usr, NULL) == 0);
        check(kill(getpid(), SIGUSR1) == 0);
        done("unblock");
        sigprocmask(MASK_UNBLOCK, &usr, NULL);
        say_line("still alive after unblocking SIGUSR1");
        return 1;
    }
    return 98;
}
