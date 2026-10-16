/* sysl code that allocates, on a bare-metal machine under QEMU: `tests/alloc_probe`'s
 * `alloc_probe_sum` fills a growable buffer, so sysl's runtime calls `malloc` and `free`, which are
 * cstd's weak defaults growing through the board's `sbrk`. That the image links at all says nothing
 * the runtime needs is left undefined. */
#include <stdlib.h>
#include <unistd.h>

void board_init(void);
void board_heap_init(void);
int alloc_probe_sum(int n);

static void say(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    if (write(1, s, n) != (ssize_t)n) {
        /* Nothing can be printed; the exit code still says it. */
    }
}

int main(void) {
    board_init();
    board_heap_init();
    int sum = alloc_probe_sum(300);
    say(sum == 44850 ? "sysl_alloc: ok\n" : "FAIL sum\n");
    exit(sum == 44850 ? 0 : 1);
}
