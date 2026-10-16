/* <unistd.h> from sh.sysl.cstd: the POSIX calls cstd makes as Linux aarch64 system calls. cstd's
 * Linux half (linux/ in the repository), the runtime a C program for keel or a static Linux program
 * links, defines each as its system call. On any other bare-metal target each is a weak default, and
 * a board replaces any one with a definition of its own: `_exit` traps, and every other sets errno to
 * ENOSYS and answers -1 (`getcwd` null, `sbrk` (void *)-1). Each answers -1 and sets errno where the
 * kernel refused. */
#ifndef CSTD_UNISTD_H
#define CSTD_UNISTD_H

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

extern char **environ;

ssize_t read(int fd, void *buf, size_t n);
ssize_t write(int fd, const void *buf, size_t n);
int close(int fd);
off_t lseek(int fd, off_t offset, int whence);
_Noreturn void _exit(int status);
pid_t getpid(void);
pid_t getppid(void);
int brk(void *addr);
void *sbrk(long increment);
int dup(int fd);
int dup2(int fd, int to);
int pipe(int fds[2]);
int unlink(const char *path);
int rmdir(const char *path);
int chdir(const char *path);
char *getcwd(char *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
