/* <sys/types.h> from sh.sysl.cstd: the widths Linux's aarch64 ABI gives each type, LP64. */
#ifndef CSTD_SYS_TYPES_H
#define CSTD_SYS_TYPES_H

#include <stddef.h>

typedef long ssize_t;
typedef long off_t;
typedef int pid_t;
typedef unsigned int mode_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef unsigned long dev_t;
typedef unsigned long ino_t;
typedef unsigned int nlink_t;
typedef int blksize_t;
typedef long blkcnt_t;
typedef long time_t;
typedef int clockid_t;

#endif
