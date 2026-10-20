/* The file calls against a real file system: `mkdir`, `chdir`, `getcwd`, `open`, `openat`, `read`,
 * `write`, `lseek`, `close`, `fstat`, `stat`, `unlink`, `rmdir`. argv[1] is a directory this run alone
 * uses, which does not exist yet; the run makes it, works inside it, and removes it. The process's
 * umask is 022. Each errno is the one POSIX names for the case, or Linux's where it documents its
 * own (unlink of a directory is EISDIR on Linux, where POSIX allows EPERM). */
#include "check.h"

#include <fcntl.h>
#include <sys/stat.h>

/* Linux's O_TMPFILE on arm64 (arch/arm64's O_DIRECTORY, 040000, with __O_TMPFILE, 020000000); no
 * cstd header names it, and `open` must still read the mode it brings. */
#define LINUX_O_TMPFILE 020040000

static int same(const char *a, const char *b) { return strcmp(a, b) == 0; }

int main(int argc, char **argv) {
    if (argc != 2) return 99;
    const char *dir = argv[1];
    char cwd[4096];
    struct stat st;
    char buf[64];

    /* mkdir */
    check(mkdir(dir, 0750) == 0);
    check_fails(mkdir(dir, 0750), EEXIST);
    check_fails(mkdir("/nonexistent-cstd/sub", 0750), ENOENT);
    check(stat(dir, &st) == 0);
    check(S_ISDIR(st.st_mode));
    check((st.st_mode & 07777) == 0750);

    /* chdir and getcwd */
    check(chdir(dir) == 0);
    check(getcwd(cwd, sizeof cwd) == cwd);
    check(same(cwd, dir));
    size_t len = strlen(dir);
    /* A buffer with no room for the terminating NUL is ERANGE; one byte more is enough. */
    errno = 0;
    check(getcwd(cwd, len) == NULL && errno == ERANGE);
    check(getcwd(cwd, len + 1) == cwd);
    errno = 0;
    check(getcwd(cwd, 1) == NULL && errno == ERANGE);
    check_fails(chdir("missing"), ENOENT);

    /* open: missing, created, the lowest free descriptor, the mode under the umask */
    check_fails(open("missing", O_RDONLY), ENOENT);
    int fd = open("file", O_WRONLY | O_CREAT | O_TRUNC, 0664);
    check(fd == 3);
    check(write(fd, "hello world", 11) == 11);
    check(write(fd, "", 0) == 0);
    check(lseek(fd, 0, SEEK_CUR) == 11);
    check(fstat(fd, &st) == 0);
    check(S_ISREG(st.st_mode));
    check((st.st_mode & 07777) == 0644);
    check(st.st_size == 11);
    check(st.st_nlink == 1);
    /* A descriptor open for writing only refuses reads with EBADF. */
    check_fails(read(fd, buf, 1), EBADF);
    check(close(fd) == 0);

    /* The same file reached by path. */
    struct stat by_path;
    check(stat("file", &by_path) == 0);
    check(by_path.st_ino == st.st_ino && by_path.st_dev == st.st_dev && by_path.st_size == 11);
    check_fails(stat("missing", &st), ENOENT);
    check_fails(stat("file/inside", &st), ENOTDIR);
    check_fails(chdir("file"), ENOTDIR);

    /* read, at the offset, to the end; lseek from each origin */
    fd = open("file", O_RDONLY);
    check(fd == 3);
    check(read(fd, buf, 5) == 5);
    check(memcmp(buf, "hello", 5) == 0);
    check(read(fd, buf, 0) == 0);
    check(lseek(fd, 6, SEEK_SET) == 6);
    memset(buf, 0, sizeof buf);
    check(read(fd, buf, sizeof buf) == 5);
    check(same(buf, "world"));
    check(read(fd, buf, sizeof buf) == 0);
    check(lseek(fd, -5, SEEK_END) == 6);
    check(lseek(fd, 1, SEEK_CUR) == 7);
    check(read(fd, buf, 1) == 1 && buf[0] == 'o');
    /* Past the end is allowed; before the start and an unknown origin are EINVAL. */
    check(lseek(fd, 100, SEEK_SET) == 100);
    check(read(fd, buf, 1) == 0);
    check_fails(lseek(fd, -1, SEEK_SET), EINVAL);
    check_fails(lseek(fd, 0, 7), EINVAL);
    check(lseek(fd, 0, SEEK_CUR) == 100);
    /* Into memory the process does not have: EFAULT. */
    check(lseek(fd, 0, SEEK_SET) == 0);
    check_fails(read(fd, (void *)8, 4), EFAULT);
    /* A read-only descriptor refuses writes with EBADF. */
    check_fails(write(fd, "x", 1), EBADF);
    check(close(fd) == 0);
    /* Every call on a closed or never-opened descriptor is EBADF. */
    check_fails(read(fd, buf, 1), EBADF);
    check_fails(write(fd, "x", 1), EBADF);
    check_fails(close(fd), EBADF);
    check_fails(close(-1), EBADF);
    check_fails(lseek(fd, 0, SEEK_SET), EBADF);
    check_fails(fstat(fd, &st), EBADF);
    check_fails(fstat(-1, &st), EBADF);
    check_fails(write(-1, "x", 1), EBADF);

    /* O_EXCL, O_APPEND, O_TRUNC, O_DIRECTORY, and a directory opened for writing */
    check_fails(open("file", O_WRONLY | O_CREAT | O_EXCL, 0600), EEXIST);
    fd = open("file", O_WRONLY | O_APPEND);
    check(fd == 3);
    check(write(fd, "!", 1) == 1);
    check(fstat(fd, &st) == 0 && st.st_size == 12);
    check(close(fd) == 0);
    check_fails(open("file", O_RDONLY | O_DIRECTORY), ENOTDIR);
    check_fails(open(".", O_WRONLY), EISDIR);
    fd = open("file", O_WRONLY | O_TRUNC);
    check(fd == 3);
    check(fstat(fd, &st) == 0 && st.st_size == 0);
    check(close(fd) == 0);

    /* openat: relative to a directory descriptor, to the working directory, and an absolute path
     * whatever the descriptor; a relative path against a bad one is EBADF, against a file ENOTDIR. */
    check(mkdir("sub", 0700) == 0);
    int sub = open("sub", O_RDONLY | O_DIRECTORY);
    check(sub == 3);
    int inner = openat(sub, "inner", O_RDWR | O_CREAT, 0600);
    check(inner == 4);
    check(write(inner, "abc", 3) == 3);
    check(close(inner) == 0);
    check(stat("sub/inner", &st) == 0 && st.st_size == 3 && (st.st_mode & 07777) == 0600);
    int cwd_relative = openat(AT_FDCWD, "sub/inner", O_RDONLY);
    check(cwd_relative == 4);
    check(read(cwd_relative, buf, 3) == 3 && memcmp(buf, "abc", 3) == 0);
    check(close(cwd_relative) == 0);
    char absolute[4200];
    strcpy(absolute, dir);
    strcat(absolute, "/file");
    int abs_fd = openat(-1, absolute, O_RDONLY);
    check(abs_fd == 4);
    check(close(abs_fd) == 0);
    check_fails(openat(-1, "file", O_RDONLY), EBADF);
    check_fails(openat(sub, "missing", O_RDONLY), ENOENT);
    int plain = open("file", O_RDONLY);
    check(plain == 4);
    check_fails(openat(plain, "x", O_RDONLY), ENOTDIR);
    check(close(plain) == 0);
    check(close(sub) == 0);

    /* O_TMPFILE: an unnamed file, given the mode it was opened with. */
    int tmp = open(".", LINUX_O_TMPFILE | O_RDWR, 0640);
    check(tmp == 3);
    check(fstat(tmp, &st) == 0);
    check(S_ISREG(st.st_mode) && (st.st_mode & 07777) == 0640 && st.st_nlink == 0);
    check(close(tmp) == 0);
    tmp = openat(AT_FDCWD, ".", LINUX_O_TMPFILE | O_RDWR, 0604);
    check(tmp == 3);
    check(fstat(tmp, &st) == 0 && (st.st_mode & 07777) == 0604);
    check(close(tmp) == 0);

    /* unlink and rmdir */
    check_fails(unlink("missing"), ENOENT);
    check_fails(unlink("sub"), EISDIR);
    check_fails(rmdir("sub"), ENOTEMPTY);
    check_fails(rmdir("file"), ENOTDIR);
    check_fails(rmdir("missing"), ENOENT);
    check(unlink("sub/inner") == 0);
    check_fails(stat("sub/inner", &st), ENOENT);
    check(rmdir("sub") == 0);
    check(unlink("file") == 0);
    check(chdir("/") == 0);
    check(getcwd(cwd, sizeof cwd) == cwd && same(cwd, "/"));
    check(rmdir(dir) == 0);
    check_fails(stat(dir, &st), ENOENT);

    return done("files");
}
