/* `dup`, `dup2` and `pipe` (POSIX dup, pipe): `dup` answers the lowest free descriptor; a duplicate
 * shares the open file description, so the offset moves for both; `dup2` closes what the target held
 * and answers the target, answers it unchanged when the two are one and it is open, and is EBADF for
 * a closed source or a target out of range; a pipe carries bytes in order from fds[1] to fds[0], and
 * reads 0 once every write end is closed. */
#include "check.h"

#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
    if (argc != 2) return 99;
    const char *file = argv[1];
    char buf[16];
    struct stat a, b;

    int fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 0600);
    check(fd == 3);
    check(write(fd, "0123456789", 10) == 10);

    /* dup */
    int copy = dup(fd);
    check(copy == 4);
    check(fstat(fd, &a) == 0 && fstat(copy, &b) == 0 && a.st_ino == b.st_ino);
    check(lseek(fd, 2, SEEK_SET) == 2);
    check(lseek(copy, 0, SEEK_CUR) == 2);
    check(read(copy, buf, 3) == 3 && memcmp(buf, "234", 3) == 0);
    check(lseek(fd, 0, SEEK_CUR) == 5);
    check_fails(dup(-1), EBADF);
    check_fails(dup(100), EBADF);
    /* Closing one leaves the other working. */
    check(close(fd) == 0);
    check(read(copy, buf, 2) == 2 && memcmp(buf, "56", 2) == 0);
    /* The freed descriptor is the lowest again. */
    fd = dup(copy);
    check(fd == 3);

    /* dup2 onto a free descriptor, onto an open one, onto itself */
    check(dup2(fd, 10) == 10);
    check(lseek(10, 0, SEEK_CUR) == 7);
    int other = open(file, O_RDONLY);
    check(other == 5);
    check(dup2(fd, other) == other);
    /* `other` now shares fd's offset rather than its own at 0. */
    check(lseek(other, 0, SEEK_CUR) == 7);
    check(dup2(fd, fd) == fd);
    check(lseek(fd, 0, SEEK_CUR) == 7);
    /* A closed source, the same closed descriptor twice, and targets out of range. */
    check_fails(dup2(20, 21), EBADF);
    check_fails(dup2(20, 20), EBADF);
    check_fails(dup2(fd, -1), EBADF);
    check_fails(dup2(fd, 1 << 30), EBADF);
    check_fails(read(21, buf, 1), EBADF);
    check(close(10) == 0);
    check(close(other) == 0);
    check(close(copy) == 0);
    check(close(fd) == 0);
    check(unlink(file) == 0);

    /* pipe: the two lowest free descriptors, bytes in order, end of file once the writer closes */
    int fds[2] = {-1, -1};
    check(pipe(fds) == 0);
    check(fds[0] == 3 && fds[1] == 4);
    check(fstat(fds[0], &a) == 0 && S_ISFIFO(a.st_mode));
    check(write(fds[1], "abc", 3) == 3);
    check(write(fds[1], "def", 3) == 3);
    check(read(fds[0], buf, 4) == 4 && memcmp(buf, "abcd", 4) == 0);
    check(read(fds[0], buf, sizeof buf) == 2 && memcmp(buf, "ef", 2) == 0);
    /* A pipe has no offset to move (POSIX lseek: ESPIPE). */
    check_fails(lseek(fds[0], 0, SEEK_SET), ESPIPE);
    /* Each end is open in one direction only. */
    check_fails(write(fds[0], "x", 1), EBADF);
    check_fails(read(fds[1], buf, 1), EBADF);
    /* A duplicate of the write end keeps the pipe open after the original closes. */
    int writer = dup(fds[1]);
    check(close(fds[1]) == 0);
    check(write(writer, "g", 1) == 1);
    check(close(writer) == 0);
    check(read(fds[0], buf, sizeof buf) == 1 && buf[0] == 'g');
    check(read(fds[0], buf, sizeof buf) == 0);
    check(close(fds[0]) == 0);
    /* fds itself in memory the process does not have. */
    check_fails(pipe((int *)8), EFAULT);

    return done("fds");
}
