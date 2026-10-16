/* cstd's operating-system defaults on a bare-metal machine under QEMU, linked with
 * `tests/board_probe`, whose C supplies a strong `write`. Everything here is printed through
 * `write`, so a line arriving at all says the board's definition is the one the link kept; cstd's
 * default would answer -1 and print nothing. Every other call is cstd's default and must answer its
 * failure value with errno ENOSYS. The program ends with SYS_EXIT_EXTENDED, its code the number of
 * failed checks; the last line is "stubs: ok" only when every check held. */
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void board_init(void);

/* `<signal.h>` does not declare it yet; `sigsetjmp` reaches it, and cstd defaults it. */
int sigprocmask(int how, const void *set, void *old);
#define SIG_SETMASK 2

static void say(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    if (write(1, s, n) != (ssize_t)n) {
        /* Nothing can be printed; the exit code still says it. */
    }
}

static void quit(int code) {
    unsigned args[2] = {0x20026, (unsigned)code};
    register int r0 __asm__("r0") = 0x20;
    register unsigned *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;) {
    }
}

static int failures;

static void check(const char *what, long got, long want) {
    int e = errno;
    if (got != want || e != ENOSYS) {
        failures++;
        say("FAIL ");
        say(what);
        say("\n");
    }
    errno = 0;
}

int main(void) {
    board_init();
    say("board write\n");

    char buf[8];
    struct stat st;
    struct timespec ts = {0, 0};
    int fds[2];

    errno = 0;
    check("read", read(0, buf, sizeof buf), -1);
    check("close", close(3), -1);
    check("lseek", lseek(3, 0, SEEK_SET), -1);
    check("open", open("f", O_RDONLY), -1);
    check("openat", openat(AT_FDCWD, "f", O_RDONLY), -1);
    check("getpid", getpid(), -1);
    check("getppid", getppid(), -1);
    check("dup", dup(0), -1);
    check("dup2", dup2(0, 1), -1);
    check("pipe", pipe(fds), -1);
    check("unlink", unlink("f"), -1);
    check("rmdir", rmdir("d"), -1);
    check("mkdir", mkdir("d", 0755), -1);
    check("chdir", chdir("d"), -1);
    check("getcwd", (long)getcwd(buf, sizeof buf), 0);
    check("fstat", fstat(0, &st), -1);
    check("stat", stat("f", &st), -1);
    check("kill", kill(1, SIGTERM), -1);
    check("sigprocmask", sigprocmask(SIG_SETMASK, 0, 0), -1);
    check("nanosleep", nanosleep(&ts, 0), -1);
    check("clock_gettime", clock_gettime(CLOCK_REALTIME, &ts), -1);
    check("brk", brk(buf), -1);
    check("sbrk", (long)sbrk(16), -1);

    if (failures == 0) say("stubs: ok\n");
    quit(failures);
    return 0;
}
