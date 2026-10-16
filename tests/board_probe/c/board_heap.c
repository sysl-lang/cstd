/* What a board with a heap region supplies: a strong `sbrk` over a static array. cstd's `malloc`
 * grows through `sbrk`, so with this in the link it has 8 KiB to allocate from; without it, cstd's
 * own `sbrk` answers ENOSYS and every allocation fails with ENOMEM.
 *
 * The file is its own archive member so a program that never calls `board_heap_init` keeps cstd's
 * `sbrk`: a static linker does not pull a member in to replace a weak definition it already has. */
#include <errno.h>
#include <stddef.h>
#include <unistd.h>

static unsigned char arena[8192] __attribute__((aligned(16)));
static size_t used;

void *sbrk(long by) {
    if (by < 0 ? (size_t)-by > used : (size_t)by > sizeof arena - used) {
        errno = ENOMEM;
        return (void *)-1;
    }
    void *old = arena + used;
    used += (size_t)by;
    return old;
}

void board_heap_init(void) {}
