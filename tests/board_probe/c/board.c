/* What a board's C supplies: a strong `write`, sending each byte to the debug console through
 * Arm semihosting's SYS_WRITEC, and an `_Exit` that ends QEMU with the status. cstd's are weak
 * defaults, so the link keeps these.
 *
 * A static linker does not pull an archive member in to replace a weak definition it already has,
 * so the program calls `board_init` too, which brings this file into the link as a board's startup
 * code would. */
#include <stdlib.h>
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

/* SYS_EXIT with ADP_Stopped_ApplicationExit and the status (SYS_EXIT_EXTENDED's argument block), so
 * QEMU exits with it. */
_Noreturn void _Exit(int status) {
    unsigned args[2] = {0x20026, (unsigned)status};
    register int r0 __asm__("r0") = 0x20;
    register unsigned *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;) {
    }
}

_Noreturn void _exit(int status) { _Exit(status); }

void board_init(void) {
#if defined(__ARM_FP)
    /* CPACR: full access to coprocessors 10 and 11, the floating-point unit, before any uses it. */
    *(volatile unsigned *)0xE000ED88 |= 0xFu << 20;
    __asm__ volatile("dsb\n isb" ::: "memory");
#endif
}
