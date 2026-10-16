/* cstd's default allocator and exit on a bare-metal machine under QEMU, linked with
 * `tests/board_probe`, whose C supplies `write`, `_Exit`, and `sbrk` over an 8 KiB array. Nothing
 * else is the board's: `malloc`, `calloc`, `realloc`, `free`, `atexit` and `exit` are cstd's weak
 * defaults. The program ends through `exit`, so the handler lines arriving after "heap: ok", last
 * registered first, say `exit` ran them before the board's `_Exit` ended the run with the number of
 * failed checks. */
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

void board_init(void);
void board_heap_init(void);

static void say(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    if (write(1, s, n) != (ssize_t)n) {
        /* Nothing can be printed; the exit code still says it. */
    }
}

static int failures;

static void check(const char *what, int ok) {
    if (!ok) {
        failures++;
        say("FAIL ");
        say(what);
        say("\n");
    }
}

static int counted;

static void count(void) { counted++; }
static void third(void) { say("atexit 3\n"); }
static void second(void) { say("atexit 2\n"); }
static void first(void) { say(counted == 29 ? "atexit 1 after 29\n" : "atexit 1 too soon\n"); }

int main(void) {
    board_init();
    board_heap_init();

    unsigned char *p = malloc(100);
    check("malloc", p != NULL && ((uintptr_t)p & 15) == 0);
    for (int i = 0; i < 100; i++) p[i] = (unsigned char)(i * 7);
    int same = 1;
    for (int i = 0; i < 100; i++) same &= p[i] == (unsigned char)(i * 7);
    check("read back", same);

    unsigned char *junk = malloc(80);
    for (int i = 0; i < 80; i++) junk[i] = 0xAA;
    free(junk);
    unsigned char *z = calloc(10, 8);
    int zero = z != NULL;
    for (int i = 0; zero && i < 80; i++) zero &= z[i] == 0;
    check("calloc reuses and zeroes", z == junk && zero);

    unsigned char *q = realloc(p, 5000);
    same = q != NULL;
    for (int i = 0; same && i < 100; i++) same &= q[i] == (unsigned char)(i * 7);
    check("realloc keeps the bytes", same);

    free(q);
    check("free then malloc reuses", malloc(5000) == q);

    errno = 0;
    check("too large", malloc(20000) == NULL && errno == ENOMEM);
    errno = 0;
    check("calloc overflow", calloc(SIZE_MAX / 2, 4) == NULL && errno == ENOMEM);
    free(z);
    free(NULL);

    check("atexit 1", atexit(first) == 0);
    check("atexit 2", atexit(second) == 0);
    check("atexit 3", atexit(third) == 0);
    for (int i = 0; i < 29; i++) check("atexit counter", atexit(count) == 0);
    check("atexit 33rd refused", atexit(count) == -1);

    if (failures == 0) say("heap: ok\n");
    exit(failures);
}
