/* cstd's own defaults on a bare-metal machine under QEMU whose board supplies nothing: `write`
 * answers -1 with errno ENOSYS, and `_Exit`, `_exit` and `abort` each stop the processor on its
 * trap instruction, which a board's fault handler sees. The handler here notes where the fault was
 * taken and why, and returns to the program's next step in place of the trap. Everything prints
 * through Arm semihosting's SYS_WRITE0; the program ends with SYS_EXIT_EXTENDED, its code the
 * number of failed checks, and the last line is "defaults: ok" only when every one held. */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void say(const char *s) {
    register int r0 __asm__("r0") = 0x04;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

static void quit(int code) {
    unsigned args[2] = {0x20026, (unsigned)code};
    register int r0 __asm__("r0") = 0x20;
    register unsigned *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;) {
    }
}

static int failures;

static void check(const char *what, int held) {
    if (!held) {
        failures++;
        say("FAIL ");
        say(what);
        say("\n");
    }
}

#define CFSR (*(volatile unsigned *)0xE000ED28)
#define HFSR (*(volatile unsigned *)0xE000ED2C)
#define VTOR (*(volatile unsigned *)0xE000ED08)
#define UNDEFINSTR (1u << 16)

/* A vector table in RAM, every exception pointing at `fault`. */
static void (*vectors[16])(void) __attribute__((aligned(512)));

static volatile unsigned fault_pc, fault_cfsr;
static volatile int step;

static void next(void);

/* The stacked frame is r0-r3, r12, lr, pc, xpsr. Notes the faulting pc and the usage-fault status,
 * clears the status, and resumes at `next` instead of the trap. */
__attribute__((used)) void fault_seen(unsigned *frame) {
    fault_pc = frame[6];
    fault_cfsr = CFSR;
    CFSR = fault_cfsr;
    HFSR = HFSR;
    frame[6] = (unsigned)(uintptr_t)next & ~1u;
}

__attribute__((naked)) static void fault(void) {
    __asm__ volatile("mrs r0, msp\n b fault_seen");
}

/* Whether `pc` is inside the first few instructions of `f`. */
static int inside(void *f, unsigned pc) {
    unsigned at = (unsigned)(uintptr_t)f & ~1u;
    return pc >= at && pc < at + 16;
}

static void next(void) {
    int s = step++;
    if (s == 1) {
        check("_Exit traps in its own body", inside((void *)_Exit, fault_pc));
        check("_Exit's trap is an undefined instruction", (fault_cfsr & UNDEFINSTR) != 0);
    } else if (s == 2) {
        check("_exit traps in its own body", inside((void *)_exit, fault_pc));
        check("_exit's trap is an undefined instruction", (fault_cfsr & UNDEFINSTR) != 0);
    } else if (s == 3) {
        check("abort traps in its own body", inside((void *)abort, fault_pc));
        check("abort's trap is an undefined instruction", (fault_cfsr & UNDEFINSTR) != 0);
    }
    fault_pc = 0;
    fault_cfsr = 0;
    if (s == 0) _Exit(3);
    if (s == 1) _exit(4);
    if (s == 2) abort();
    if (failures == 0) say("defaults: ok\n");
    quit(failures);
}

int main(void) {
#if defined(__ARM_FP)
    *(volatile unsigned *)0xE000ED88 |= 0xFu << 20;
    __asm__ volatile("dsb\n isb" ::: "memory");
#endif
    for (int i = 0; i < 16; i++) vectors[i] = fault;
    VTOR = (unsigned)(uintptr_t)vectors;
    __asm__ volatile("dsb\n isb" ::: "memory");

    errno = 0;
    check("write answers -1", write(1, "x", 1) == -1);
    check("write sets errno to ENOSYS", errno == ENOSYS);
    errno = 0;
    check("write of no bytes answers -1 with ENOSYS too", write(2, "", 0) == -1 && errno == ENOSYS);

    errno = 0;
    check("puts answers EOF", puts("x") == EOF);
    check("puts sets errno to ENOSYS", errno == ENOSYS);
    errno = 0;
    check("putchar answers EOF", putchar('x') == EOF);
    check("putchar sets errno to ENOSYS", errno == ENOSYS);

    errno = 0;
    check("getenv answers null on a board", getenv("PATH") == NULL);
    check("getenv leaves errno alone", errno == 0);

    next();
    return 0;
}
