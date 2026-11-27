/* cstd's default allocator on a bare-metal machine under QEMU whose board supplies no `sbrk`: linked
 * with `tests/board_probe` but never calling `board_heap_init`, so the board's `sbrk` stays out of
 * the link and cstd's ENOSYS default answers. Every allocation must fail with ENOMEM, and so must a
 * float conversion through vsnprintf, whose specifier is the one thing the printf family allocates; a
 * conversion that allocates nothing still prints. */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
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

static int into(char *t, size_t n, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(t, n, fmt, ap);
    va_end(ap);
    return r;
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
    char text[16];
    errno = 0;
    if (into(text, sizeof text, "%.2f", 1.5) != -1 || errno != ENOMEM) {
        failures++;
        say("FAIL vsnprintf %f\n");
    }
    errno = 0;
    if (into(text, sizeof text, "%d", 42) != 2 || errno != 0 || text[0] != '4' || text[1] != '2' || text[2] != 0) {
        failures++;
        say("FAIL vsnprintf %d\n");
    }
    if (failures == 0) say("nosbrk: ok\n");
    _Exit(failures);
}
