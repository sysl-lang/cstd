/* cstd's default allocator on a bare-metal machine under QEMU whose board supplies no `sbrk`: linked
 * with `tests/board_probe` but never calling `board_heap_init`, so the board's `sbrk` stays out of
 * the link and cstd's ENOSYS default answers. Every allocation must fail with ENOMEM. */
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

void board_init(void);

static void say(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    if (write(1, s, n) != (ssize_t)n) {
        /* Nothing can be printed; the exit code still says it. */
    }
}

int main(void) {
    board_init();
    int failures = 0;
    errno = 0;
    if (malloc(16) != NULL || errno != ENOMEM) {
        failures++;
        say("FAIL malloc\n");
    }
    errno = 0;
    if (calloc(4, 4) != NULL || errno != ENOMEM) {
        failures++;
        say("FAIL calloc\n");
    }
    errno = 0;
    if (realloc(NULL, 16) != NULL || errno != ENOMEM) {
        failures++;
        say("FAIL realloc\n");
    }
    if (failures == 0) say("nosbrk: ok\n");
    _Exit(failures);
}
