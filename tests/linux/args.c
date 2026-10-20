/* `_start` and `cstd_start`: Linux's initial stack read into `main`'s argc, argv and envp, `environ`
 * pointing at the same vector, both arrays ended by a null (C 5.1.2.2.1, POSIX exec), and main's
 * answer becoming the process's exit status through `exit`. Run as `args one "two words" ""` with
 * exactly `A=1` and `B=two words` in its environment. */
#include "check.h"

#include <stdlib.h>

extern char **environ;

int main(int argc, char **argv, char **envp) {
    check(argc == 4);
    check(argv[argc] == NULL);
    check(strcmp(argv[1], "one") == 0);
    check(strcmp(argv[2], "two words") == 0);
    check(strcmp(argv[3], "") == 0);
    /* argv[0] is the path the program was run by. */
    const char *base = strrchr(argv[0], '/');
    check(base != NULL && strcmp(base, "/args") == 0);

    check(environ == envp);
    check(strcmp(envp[0], "A=1") == 0);
    check(strcmp(envp[1], "B=two words") == 0);
    check(envp[2] == NULL);
    /* The environment follows argv's terminating null on the stack, as exec lays them out. */
    check(envp == argv + argc + 1);

    if (done("args") != 0) return 1;
    /* Returning from main is calling exit with the value (C 5.1.2.2.3). */
    return 42;
}
