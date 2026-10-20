/* `creat` and `fcntl` (POSIX creat, fcntl). `creat` is `open` with `O_WRONLY | O_CREAT | O_TRUNC`: it
 * makes the file, empties an existing one, gives it the mode less the umask (022 here), opens it
 * write-only, and is ENOENT under a missing directory. `fcntl`: `F_DUPFD` answers the lowest free
 * descriptor not less than its argument, without close-on-exec, and `F_DUPFD_CLOEXEC` with it;
 * `F_GETFD`/`F_SETFD` read and write `FD_CLOEXEC`; `F_GETFL`/`F_SETFL` read the status flags and
 * write `O_NONBLOCK` and `O_APPEND` but not the access mode; a bad descriptor is EBADF, a command
 * that does not exist EINVAL; `F_SETLK` and `F_GETLK` take a `struct flock`. */
#include "check.h"

#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
    if (argc != 2) return 99;
    const char *dir = argv[1];
    char file[256];
    size_t n = strlen(dir);
    memcpy(file, dir, n);
    memcpy(file + n, "/f", 3);
    struct stat st;
    char buf[16];

    check(mkdir(dir, 0700) == 0);

    /* creat */
    int fd = creat(file, 0666);
    check(fd == 3);
    check(stat(file, &st) == 0 && S_ISREG(st.st_mode) && (st.st_mode & 07777) == 0644 && st.st_size == 0);
    check(write(fd, "hello", 5) == 5);
    check_fails(read(fd, buf, 1), EBADF);
    check((fcntl(fd, F_GETFL) & O_ACCMODE) == O_WRONLY);
    check(close(fd) == 0);
    check(stat(file, &st) == 0 && st.st_size == 5);
    /* An existing file is truncated, and keeps its own mode. */
    fd = creat(file, 0777);
    check(fd == 3);
    check(stat(file, &st) == 0 && st.st_size == 0 && (st.st_mode & 07777) == 0644);
    check(close(fd) == 0);
    fd = creat("/tmp/cstd-creat-mode-probe", 0640);
    check(fd == 3 && fstat(fd, &st) == 0 && (st.st_mode & 07777) == 0640);
    check(close(fd) == 0 && unlink("/tmp/cstd-creat-mode-probe") == 0);
    check_fails(creat("/nonexistent-cstd/f", 0644), ENOENT);

    /* F_DUPFD */
    fd = open(file, O_RDWR);
    check(fd == 3);
    int d = fcntl(fd, F_DUPFD, 10);
    check(d == 10);
    check(fcntl(d, F_GETFD) == 0);
    d = fcntl(fd, F_DUPFD, 10);
    check(d == 11);
    check(fcntl(fd, F_DUPFD, 0) == 4);
    check_fails(fcntl(fd, F_DUPFD, -1), EINVAL);
    /* The copy shares the description: a status flag set through one reads back through the other. */
    check(fcntl(10, F_SETFL, O_NONBLOCK) == 0);
    check((fcntl(fd, F_GETFL) & O_NONBLOCK) != 0);

    /* F_DUPFD_CLOEXEC and F_GETFD / F_SETFD */
    int c = fcntl(fd, F_DUPFD_CLOEXEC, 20);
    check(c == 20);
    check(fcntl(c, F_GETFD) == FD_CLOEXEC);
    check(fcntl(fd, F_GETFD) == 0);
    check(fcntl(fd, F_SETFD, FD_CLOEXEC) == 0);
    check(fcntl(fd, F_GETFD) == FD_CLOEXEC);
    check(fcntl(fd, F_SETFD, 0) == 0);
    check(fcntl(fd, F_GETFD) == 0);

    /* F_GETFL and F_SETFL: the access mode stays, O_NONBLOCK and O_APPEND are written. */
    int d2 = open(file, O_RDWR);
    check((fcntl(d2, F_GETFL) & O_ACCMODE) == O_RDWR);
    check((fcntl(d2, F_GETFL) & O_NONBLOCK) == 0);
    check(fcntl(d2, F_SETFL, O_NONBLOCK | O_APPEND) == 0);
    int fl = fcntl(d2, F_GETFL);
    check((fl & O_NONBLOCK) != 0 && (fl & O_APPEND) != 0 && (fl & O_ACCMODE) == O_RDWR);
    check(fcntl(d2, F_SETFL, 0) == 0);
    fl = fcntl(d2, F_GETFL);
    check((fl & O_NONBLOCK) == 0 && (fl & O_APPEND) == 0 && (fl & O_ACCMODE) == O_RDWR);

    /* Errors */
    check_fails(fcntl(99, F_GETFD), EBADF);
    check_fails(fcntl(-1, F_GETFL), EBADF);
    check_fails(fcntl(99, F_DUPFD, 0), EBADF);
    check_fails(fcntl(fd, 0x7fff), EINVAL);
    check_fails(fcntl(fd, -5), EINVAL);

    /* Locks: a lock this process holds is no conflict for it, and the read/write descriptor modes
     * must agree with the lock's type. */
    struct flock fk;
    memset(&fk, 0, sizeof fk);
    fk.l_type = F_WRLCK;
    fk.l_whence = SEEK_SET;
    fk.l_start = 0;
    fk.l_len = 0;
    check(fcntl(d2, F_SETLK, &fk) == 0);
    memset(&fk, 0, sizeof fk);
    fk.l_type = F_WRLCK;
    fk.l_whence = SEEK_SET;
    check(fcntl(d2, F_GETLK, &fk) == 0);
    check(fk.l_type == F_UNLCK);
    fk.l_type = F_UNLCK;
    check(fcntl(d2, F_SETLK, &fk) == 0);
    check(fcntl(d2, F_SETLKW, &fk) == 0);
    int ro = open(file, O_RDONLY);
    fk.l_type = F_WRLCK;
    check_fails(fcntl(ro, F_SETLK, &fk), EBADF);
    fk.l_type = F_RDLCK;
    check(fcntl(ro, F_SETLK, &fk) == 0);
    check(sizeof(struct flock) == 32);

    return done("fcntl");
}
