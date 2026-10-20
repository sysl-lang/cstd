/* <fcntl.h> from sh.sysl.cstd: creat, fcntl, open and openat, with the flag values of Linux's arm64 ABI (which
 * differ from the generic ones for O_DIRECTORY and O_NOFOLLOW). Defined by cstd's Linux half; on any
 * other bare-metal target each is a weak default answering -1 with errno ENOSYS, which a board
 * replaces with a definition of its own. */
#ifndef CSTD_FCNTL_H
#define CSTD_FCNTL_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_ACCMODE 03
#define O_CREAT 0100
#define O_EXCL 0200
#define O_NOCTTY 0400
#define O_TRUNC 01000
#define O_APPEND 02000
#define O_NONBLOCK 04000
#define O_DIRECTORY 040000
#define O_NOFOLLOW 0100000
#define O_CLOEXEC 02000000

#define AT_FDCWD (-100)
#define AT_REMOVEDIR 0x200

#define F_DUPFD 0
#define F_GETFD 1
#define F_SETFD 2
#define F_GETFL 3
#define F_SETFL 4
#define F_GETLK 5
#define F_SETLK 6
#define F_SETLKW 7
#define F_DUPFD_CLOEXEC 1030
#define FD_CLOEXEC 1

#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2

struct flock {
    short l_type;
    short l_whence;
    off_t l_start;
    off_t l_len;
    pid_t l_pid;
};

int creat(const char *path, mode_t mode);
int fcntl(int fd, int cmd, ...);
int open(const char *path, int flags, ...);
int openat(int dirfd, const char *path, int flags, ...);

#ifdef __cplusplus
}
#endif

#endif
