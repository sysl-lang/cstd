/* What a board's C supplies: a strong `write`, sending each byte to the debug console through
 * Arm semihosting's SYS_WRITEC. cstd's `write` is a weak default, so the link keeps this one.
 *
 * A static linker does not pull an archive member in to replace a weak definition it already has,
 * so the program calls `board_init` too, which brings this file into the link as a board's startup
 * code would. */
#include <unistd.h>

static void writec(char c) {
    register int r0 __asm__("r0") = 0x03;
    register const char *r1 __asm__("r1") = &c;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

ssize_t write(int fd, const void *buf, size_t n) {
    (void)fd;
    const char *s = buf;
    for (size_t i = 0; i < n; i++) writec(s[i]);
    return (ssize_t)n;
}

void board_init(void) {
#if defined(__ARM_FP)
    /* CPACR: full access to coprocessors 10 and 11, the floating-point unit, before any uses it. */
    *(volatile unsigned *)0xE000ED88 |= 0xFu << 20;
    __asm__ volatile("dsb\n isb" ::: "memory");
#endif
}
