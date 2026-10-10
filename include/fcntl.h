/* <fcntl.h> from sh.sysl.cstd: open and openat, with the flag values of Linux's arm64 ABI (which
 * differ from the generic ones for O_DIRECTORY and O_NOFOLLOW). Defined by cstd's Linux half. */
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

int open(const char *path, int flags, ...);
int openat(int dirfd, const char *path, int flags, ...);

#ifdef __cplusplus
}
#endif

#endif
