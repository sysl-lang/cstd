/* `malloc`, `calloc`, `realloc` and `free` over Linux's break (C 7.24.3, POSIX malloc): storage
 * suitably aligned for any object, distinct blocks that do not overlap, `calloc`'s zeros, `realloc`
 * keeping the bytes up to the smaller size, `free(NULL)` doing nothing, and null with ENOMEM where
 * the space cannot be had. */
#include "check.h"

#include <stdint.h>
#include <stdlib.h>

static int filled(const unsigned char *p, size_t n, unsigned char seed) {
    for (size_t i = 0; i < n; i++)
        if (p[i] != (unsigned char)(seed + i * 7)) return 0;
    return 1;
}

static void fill(unsigned char *p, size_t n, unsigned char seed) {
    for (size_t i = 0; i < n; i++) p[i] = (unsigned char)(seed + i * 7);
}

int main(void) {
    /* malloc(0) answers null or a pointer free accepts (C 7.24.3 ¶1); two such pointers are distinct. */
    void *zero_a = malloc(0);
    void *zero_b = malloc(0);
    check(zero_a == NULL || zero_a != zero_b);
    free(zero_a);
    free(zero_b);

    free(NULL);

    /* Alignment: max_align_t is 16 on aarch64, and every block honours it. */
    for (size_t n = 1; n <= 129; n += 8) {
        void *p = malloc(n);
        check(p != NULL);
        check(((uintptr_t)p % _Alignof(max_align_t)) == 0);
        free(p);
    }

    /* Many live blocks of different sizes, each holding its own bytes. */
    enum { COUNT = 400 };
    unsigned char *blocks[COUNT];
    size_t sizes[COUNT];
    for (int i = 0; i < COUNT; i++) {
        sizes[i] = (size_t)(1 + (i * 37) % 700);
        blocks[i] = malloc(sizes[i]);
        check(blocks[i] != NULL);
        fill(blocks[i], sizes[i], (unsigned char)i);
    }
    for (int i = 0; i < COUNT; i++) check(filled(blocks[i], sizes[i], (unsigned char)i));
    /* Free every other block, allocate into the holes, and the survivors are untouched. */
    for (int i = 0; i < COUNT; i += 2) free(blocks[i]);
    for (int i = 0; i < COUNT; i += 2) {
        blocks[i] = malloc(sizes[i]);
        check(blocks[i] != NULL);
        fill(blocks[i], sizes[i], (unsigned char)(i + 1));
    }
    for (int i = 0; i < COUNT; i++)
        check(filled(blocks[i], sizes[i], (unsigned char)(i % 2 == 0 ? i + 1 : i)));
    for (int i = COUNT - 1; i >= 0; i--) free(blocks[i]);

    /* A large block: 64 MiB, far past one growth of the break, writable at both ends. */
    size_t big = (size_t)64 << 20;
    unsigned char *large = malloc(big);
    check(large != NULL);
    if (large != NULL) {
        large[0] = 0xa5;
        large[big - 1] = 0x5a;
        check(large[0] == 0xa5 && large[big - 1] == 0x5a);
        free(large);
    }

    /* Space that cannot be had: null and ENOMEM (POSIX malloc, calloc, realloc). */
    errno = 0;
    check(malloc(SIZE_MAX) == NULL && errno == ENOMEM);
    errno = 0;
    check(malloc(SIZE_MAX / 2) == NULL && errno == ENOMEM);
    errno = 0;
    check(malloc((size_t)1 << 46) == NULL && errno == ENOMEM);
    /* The heap still works after a refusal. */
    void *after = malloc(100);
    check(after != NULL);
    free(after);

    /* calloc: zeroed, including a block reused from one that held bytes. */
    unsigned char *dirty = malloc(4096);
    check(dirty != NULL);
    memset(dirty, 0xff, 4096);
    free(dirty);
    unsigned char *zeros = calloc(512, 8);
    check(zeros != NULL);
    int all_zero = 1;
    for (int i = 0; i < 4096; i++)
        if (zeros[i] != 0) all_zero = 0;
    check(all_zero);
    free(zeros);
    /* count * size overflowing is refused, not wrapped (C 7.24.3.2). */
    errno = 0;
    check(calloc(SIZE_MAX / 2, 3) == NULL && errno == ENOMEM);
    errno = 0;
    check(calloc((size_t)1 << 40, (size_t)1 << 30) == NULL && errno == ENOMEM);

    /* realloc(NULL, n) is malloc(n) (C 7.24.3.7). */
    unsigned char *r = realloc(NULL, 10);
    check(r != NULL);
    fill(r, 10, 3);
    /* Growth keeps the old bytes, across a move: a block allocated behind it stops it growing in place. */
    void *fence = malloc(32);
    unsigned char *grown = realloc(r, 100000);
    check(grown != NULL);
    if (grown != NULL) {
        check(filled(grown, 10, 3));
        fill(grown, 100000, 9);
        /* Shrinking keeps the prefix. */
        unsigned char *shrunk = realloc(grown, 50);
        check(shrunk != NULL);
        if (shrunk != NULL) {
            check(filled(shrunk, 50, 9));
            /* A refused realloc leaves the old block where it was, intact (C 7.24.3.7 ¶3). */
            errno = 0;
            check(realloc(shrunk, SIZE_MAX) == NULL && errno == ENOMEM);
            check(filled(shrunk, 50, 9));
            free(shrunk);
        }
    }
    free(fence);

    /* A thousand rounds of growth by small steps keeps every byte. */
    unsigned char *step = NULL;
    for (size_t n = 1; n <= 1000; n++) {
        unsigned char *next = realloc(step, n);
        check(next != NULL);
        if (next == NULL) break;
        next[n - 1] = (unsigned char)(n * 7);
        step = next;
    }
    int kept = 1;
    for (size_t n = 1; n <= 1000; n++)
        if (step[n - 1] != (unsigned char)(n * 7)) kept = 0;
    check(kept);
    free(step);

    return done("heap");
}
