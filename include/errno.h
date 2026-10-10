/* <errno.h> from sh.sysl.cstd. errno is one global object, defined by the library. */
#ifndef CSTD_ERRNO_H
#define CSTD_ERRNO_H

#ifdef __cplusplus
extern "C" {
#endif

extern int errno;

#define EINVAL 22
#define ERANGE 34

#ifdef __cplusplus
}
#endif

#endif
