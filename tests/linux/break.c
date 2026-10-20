/* `brk` and `sbrk` before anything has allocated (the legacy interfaces as Linux documents them in
 * brk(2)): `sbrk(0)` answers the current break, `sbrk(n)` answers the old break and moves it by n,
 * a negative increment gives memory back, `brk` sets the break and answers 0; a break the kernel
 * will not grant is -1 (`(void *)-1` from `sbrk`) with ENOMEM, and the break is left where it was. */
#include "check.h"

#include <stdint.h>

int main(void) {
    char *start = sbrk(0);
    check(start != (char *)-1);
    check(sbrk(0) == start);

    char *old = sbrk(4096);
    check(old == start);
    check(sbrk(0) == start + 4096);
    /* The new space is memory the program may write. */
    memset(start, 0x5c, 4096);
    check(start[0] == 0x5c && start[4095] == 0x5c);

    check(sbrk(8192) == start + 4096);
    check(sbrk(0) == start + 4096 + 8192);
    check(sbrk(-8192) == start + 4096 + 8192);
    check(sbrk(0) == start + 4096);

    check(brk(start + 65536) == 0);
    check(sbrk(0) == start + 65536);
    start[65535] = 1;
    check(start[65535] == 1);
    check(brk(start) == 0);
    check(sbrk(0) == start);

    /* Below the program's own data, and far past any memory: refused, the break unmoved. */
    check_fails(brk((void *)4096), ENOMEM);
    check(sbrk(0) == start);
    check_fails(brk(start + ((intptr_t)1 << 46)), ENOMEM);
    check(sbrk(0) == start);

    errno = 0;
    check(sbrk((intptr_t)1 << 46) == (void *)-1 && errno == ENOMEM);
    check(sbrk(0) == start);
    errno = 0;
    check(sbrk(INTPTR_MAX) == (void *)-1 && errno == ENOMEM);
    check(sbrk(0) == start);
    errno = 0;
    check(sbrk(-(intptr_t)((uintptr_t)start + 4096)) == (void *)-1 && errno == ENOMEM);
    check(sbrk(0) == start);

    return done("break");
}
