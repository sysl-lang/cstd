/* `getenv` (POSIX getenv): the value after `NAME=` of the entry whose name matches exactly, a null
 * pointer where there is none; a name holding `=` or empty names nothing. The test runs this under
 * `env -i HOME=/home/x HOMEX=/other EMPTY= LONGER_NAME=1 A=1`. */
#include "check.h"

#include <stdlib.h>

extern char **environ;

int main(int argc, char **argv) {
    /* With an empty environment there is nothing to find. */
    if (argc == 2) {
        check(environ != NULL && environ[0] == NULL);
        check(getenv("HOME") == NULL);
        return done("env-empty");
    }
    char *v = getenv("HOME");
    check(v != NULL && strcmp(v, "/home/x") == 0);
    v = getenv("HOMEX");
    check(v != NULL && strcmp(v, "/other") == 0);
    /* A name that is a prefix of another entry's name, or has one as its prefix, is not a match. */
    check(getenv("HOM") == NULL);
    check(getenv("HOMEXX") == NULL);
    check(getenv("LONGER") == NULL);
    /* An empty value is a pointer to an empty string, not null. */
    v = getenv("EMPTY");
    check(v != NULL && v[0] == 0);
    v = getenv("A");
    check(v != NULL && strcmp(v, "1") == 0);
    check(getenv("MISSING") == NULL);
    check(getenv("") == NULL);
    check(getenv("HOME=/home/x") == NULL);
    check(getenv("HOME=") == NULL);
    check(getenv("=") == NULL);
    /* The value is in the environment's own storage, just past the `=`. */
    int found = 0;
    for (char **e = environ; *e != NULL; e++)
        if (strncmp(*e, "HOME=", 5) == 0) {
            check(getenv("HOME") == *e + 5);
            found = 1;
        }
    check(found);
    return done("env");
}
