/* The pseudo-terminal functions (POSIX posix_openpt, grantpt, unlockpt, ptsname, ptsname_r) on
 * Linux's devpts: a master from /dev/ptmx, a slave named /dev/pts/N that is locked until unlockpt,
 * bytes crossing in both directions through the line discipline (cooked: canonical input, echo and
 * NL to CR NL on output; raw: untouched), end of the slave answering the master with EIO, two pairs
 * independent, every function's POSIX errors, and O_NOCTTY keeping a session leader's controlling
 * terminal from being taken.
 *
 * cstd has no <termios.h> or ioctl, so the raw-mode switch and setsid are system calls made here
 * directly; the line discipline is the kernel's and is what is being observed. */
#include "check.h"

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

static long sys(long nr, long a, long b, long c) {
    register long x8 __asm__("x8") = nr;
    register long x0 __asm__("x0") = a;
    register long x1 __asm__("x1") = b;
    register long x2 __asm__("x2") = c;
    __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");
    return x0;
}

#define SYS_IOCTL 29
#define SYS_SETSID 157
#define TCGETS 0x5401
#define TCSETS 0x5402
#define ICANON 0000002
#define ECHO 0000010
#define CS8 0000060
#define VTIME 5
#define VMIN 6

/* The kernel's struct termios on arm64. */
struct ktermios {
    unsigned c_iflag, c_oflag, c_cflag, c_lflag;
    unsigned char c_line, c_cc[19];
};

static int make_raw(int fd) {
    struct ktermios t;
    if (sys(SYS_IOCTL, fd, TCGETS, (long)&t) != 0) return -1;
    t.c_iflag = 0;
    t.c_oflag = 0;
    t.c_lflag = 0;
    t.c_cflag = (t.c_cflag & ~0000060u) | CS8;
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    return (int)sys(SYS_IOCTL, fd, TCSETS, (long)&t);
}

static unsigned local_flags(int fd) {
    struct ktermios t;
    if (sys(SYS_IOCTL, fd, TCGETS, (long)&t) != 0) return 0xffffffffu;
    return t.c_lflag;
}

static void pause_a_moment(void) {
    struct timespec ts = {0, 100 * 1000 * 1000};
    nanosleep(&ts, 0);
}

/* Reads until `n` bytes have arrived, which one write can split across reads. */
static int read_exactly(int fd, char *buf, int n) {
    int got = 0;
    while (got < n) {
        long r = read(fd, buf + got, (size_t)(n - got));
        if (r <= 0) return -1;
        got += (int)r;
    }
    return got;
}

static void set_nonblock(int fd, int on) {
    int fl = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, on ? (fl | O_NONBLOCK) : (fl & ~O_NONBLOCK));
}

static int starts_with(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

int main(void) {
    char buf[64];
    char name[64];
    struct stat st;

    /* A master: a character device, locked slave, a name that exists. */
    int master = posix_openpt(O_RDWR | O_NOCTTY);
    check(master >= 0);
    check(fstat(master, &st) == 0 && S_ISCHR(st.st_mode));
    check(grantpt(master) == 0);
    char *p = ptsname(master);
    check(p != 0 && starts_with(p, "/dev/pts/") && strlen(p) > 9);
    if (p == 0) return done("pty");
    strcpy(name, p);
    check(stat(name, &st) == 0 && S_ISCHR(st.st_mode));
    /* A slave opened before unlockpt is refused. */
    check_fails(open(name, O_RDWR | O_NOCTTY), EIO);
    check(unlockpt(master) == 0);
    int slave = open(name, O_RDWR | O_NOCTTY);
    check(slave >= 0);
    check(fstat(slave, &st) == 0 && S_ISCHR(st.st_mode));
    /* unlockpt twice is harmless. */
    check(unlockpt(master) == 0);

    /* Cooked: the slave sees a line only once it is complete, the master sees the echo, and the
     * slave's newline goes out as CR NL. */
    check((local_flags(slave) & (ICANON | ECHO)) == (ICANON | ECHO));
    check(write(master, "abc", 3) == 3);
    pause_a_moment();
    set_nonblock(slave, 1);
    check_fails(read(slave, buf, sizeof buf), EAGAIN);
    set_nonblock(slave, 0);
    check(write(master, "\n", 1) == 1);
    check(read_exactly(slave, buf, 4) == 4 && memcmp(buf, "abc\n", 4) == 0);
    check(read_exactly(master, buf, 5) == 5 && memcmp(buf, "abc\r\n", 5) == 0);
    check(write(slave, "hi\n", 3) == 3);
    check(read_exactly(master, buf, 4) == 4 && memcmp(buf, "hi\r\n", 4) == 0);

    /* Raw, for a terminal emulator: bytes cross unchanged both ways and nothing is echoed. */
    check(make_raw(slave) == 0);
    check((local_flags(slave) & (ICANON | ECHO)) == 0);
    check(write(master, "xyz", 3) == 3);
    check(read_exactly(slave, buf, 3) == 3 && memcmp(buf, "xyz", 3) == 0);
    check(write(master, "a\r\n\x03", 4) == 4);
    check(read_exactly(slave, buf, 4) == 4 && memcmp(buf, "a\r\n\x03", 4) == 0);
    check(write(slave, "a\nb", 3) == 3);
    check(read_exactly(master, buf, 3) == 3 && memcmp(buf, "a\nb", 3) == 0);
    pause_a_moment();
    set_nonblock(master, 1);
    check_fails(read(master, buf, sizeof buf), EAGAIN);
    set_nonblock(master, 0);

    /* The slave closed: the master reads EIO, not end of file, and the pair is still there to be
     * opened again by name. */
    check(close(slave) == 0);
    check_fails(read(master, buf, sizeof buf), EIO);
    slave = open(name, O_RDWR | O_NOCTTY);
    check(slave >= 0);
    check(write(slave, "again", 5) == 5);
    check(read_exactly(master, buf, 5) == 5 && memcmp(buf, "again", 5) == 0);
    check(close(slave) == 0);

    /* ptsname_r: the same name, an exact fit, one byte short, none at all, no buffer. The buffer is
     * left as it was on ERANGE, and errno is never set (the error is the answer). */
    size_t need = strlen(name) + 1;
    memset(buf, 'Q', sizeof buf);
    errno = 0;
    check(ptsname_r(master, buf, need) == 0 && strcmp(buf, name) == 0 && errno == 0);
    memset(buf, 'Q', sizeof buf);
    check(ptsname_r(master, buf, need - 1) == ERANGE && buf[0] == 'Q' && errno == 0);
    check(ptsname_r(master, buf, 1) == ERANGE && buf[0] == 'Q');
    check(ptsname_r(master, buf, 0) == ERANGE && buf[0] == 'Q');
    check(ptsname_r(master, 0, 64) == EINVAL);

    /* Two pairs at once: distinct names, distinct devices, each slave's output on its own master. */
    int master2 = posix_openpt(O_RDWR | O_NOCTTY);
    check(master2 >= 0 && master2 != master);
    check(grantpt(master2) == 0 && unlockpt(master2) == 0);
    char name2[64];
    check(ptsname_r(master2, name2, sizeof name2) == 0);
    check(starts_with(name2, "/dev/pts/") && strcmp(name, name2) != 0);
    struct stat st2;
    check(stat(name, &st) == 0 && stat(name2, &st2) == 0 && st.st_rdev != st2.st_rdev);
    int slave1 = open(name, O_RDWR | O_NOCTTY);
    int slave2 = open(name2, O_RDWR | O_NOCTTY);
    check(slave1 >= 0 && slave2 >= 0);
    check(write(slave1, "A", 1) == 1);
    check(read_exactly(master, buf, 1) == 1 && buf[0] == 'A');
    pause_a_moment();
    set_nonblock(master2, 1);
    check_fails(read(master2, buf, sizeof buf), EAGAIN);
    set_nonblock(master2, 0);
    check(write(slave2, "B", 1) == 1);
    check(read_exactly(master2, buf, 1) == 1 && buf[0] == 'B');
    /* ptsname's one buffer is shared: the second call replaces the first's text. */
    char *first = ptsname(master);
    char *second = ptsname(master2);
    check(first == second && strcmp(second, name2) == 0);
    check(close(slave1) == 0 && close(slave2) == 0);
    check(close(master2) == 0);

    /* Errors. An unopen descriptor is EBADF from every function; a descriptor that is not a master
     * (a character device that is not a terminal, a slave, a pipe) is EINVAL from grantpt and
     * unlockpt, ENOTTY from the name functions. */
    int bad[] = {-1, 1000};
    for (int i = 0; i < 2; i++) {
        check_fails(grantpt(bad[i]), EBADF);
        check_fails(unlockpt(bad[i]), EBADF);
        errno = 0;
        check(ptsname(bad[i]) == 0 && errno == EBADF);
        errno = 0;
        check(ptsname_r(bad[i], buf, sizeof buf) == EBADF && errno == 0);
    }
    int null_fd = open("/dev/null", O_RDWR);
    int fds[2];
    check(null_fd >= 0 && pipe(fds) == 0);
    slave = open(name, O_RDWR | O_NOCTTY);
    int others[] = {null_fd, fds[0], slave};
    for (int i = 0; i < 3; i++) {
        check_fails(grantpt(others[i]), EINVAL);
        check_fails(unlockpt(others[i]), EINVAL);
        errno = 0;
        check(ptsname(others[i]) == 0 && errno == ENOTTY);
        errno = 0;
        check(ptsname_r(others[i], buf, sizeof buf) == ENOTTY && errno == 0);
    }
    check(close(slave) == 0 && close(null_fd) == 0 && close(fds[0]) == 0 && close(fds[1]) == 0);
    /* A closed master is a bad descriptor. */
    int dead = master;
    check(close(master) == 0);
    check_fails(grantpt(dead), EBADF);
    check_fails(unlockpt(dead), EBADF);
    errno = 0;
    check(ptsname(dead) == 0 && errno == EBADF);
    /* posix_openpt passes the flags to the open: a flag /dev/ptmx cannot take is its error. */
    check_fails(posix_openpt(O_RDWR | O_DIRECTORY), ENOTDIR);

    /* O_NOCTTY. A session leader with no controlling terminal takes the first terminal it opens
     * without O_NOCTTY as its own, and /dev/tty then opens; with O_NOCTTY on every open it never
     * does, and /dev/tty stays ENXIO. */
    check(sys(SYS_SETSID, 0, 0, 0) > 0);
    /* Closing the master of a controlling terminal hangs the session up; SIGHUP is blocked so the
     * program lives to report. */
    unsigned long hup = 1UL << 0;
    check(sigprocmask(0, (void *)&hup, 0) == 0);
    check_fails(open("/dev/tty", O_RDWR), ENXIO);
    int m = posix_openpt(O_RDWR | O_NOCTTY);
    check(m >= 0 && grantpt(m) == 0 && unlockpt(m) == 0);
    check(ptsname_r(m, name, sizeof name) == 0);
    int s = open(name, O_RDWR | O_NOCTTY);
    check(s >= 0);
    check_fails(open("/dev/tty", O_RDWR), ENXIO);
    check(close(s) == 0);
    s = open(name, O_RDWR);
    check(s >= 0);
    int tty = open("/dev/tty", O_RDWR);
    check(tty >= 0);
    check(close(tty) == 0 && close(s) == 0 && close(m) == 0);

    return done("pty");
}
