/* cstd's setjmp on a bare-metal machine under QEMU: a Cortex-M0, a Cortex-M3, and an RV32 core.
 * It reports through Arm/RISC-V semihosting and ends with SYS_EXIT_EXTENDED, its code the number
 * of failed checks. The last line is "setjmp: ok" only when every check held. */
#include <setjmp.h>

static void semihost(int op, void *arg) {
#if defined(__thumb__)
    register int r0 __asm__("r0") = op;
    register void *r1 __asm__("r1") = arg;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
#else
    register long a0 __asm__("a0") = op;
    register void *a1 __asm__("a1") = arg;
    __asm__ volatile(".option push\n .option norvc\n slli zero, zero, 0x1f\n ebreak\n"
                     " srai zero, zero, 7\n .option pop"
                     : "+r"(a0) : "r"(a1) : "memory");
#endif
}

static void say(const char *s) { semihost(0x04, (void *)s); }

static void say_hex(unsigned long v) {
    char out[12];
    int k = 0;
    out[k++] = '0';
    out[k++] = 'x';
    for (int shift = 28; shift >= 0; shift -= 4) out[k++] = "0123456789abcdef"[(v >> shift) & 15];
    out[k++] = '\n';
    out[k] = 0;
    say(out);
}

static void quit(int code) {
    unsigned args[2] = {0x20026, (unsigned)code};
    semihost(0x20, args);
    for (;;) {
    }
}

static jmp_buf env;
static int failures;

static void check(const char *what, long got, long want) {
    if (got != want) {
        failures++;
        say("FAIL ");
        say(what);
        say(" got ");
        say_hex((unsigned long)got);
    }
}

/* A setjmp that answered 0 after a jump would jump again for ever, so the chain gives up. */
static int jumps;

__attribute__((noinline)) static void third(int v) {
    if (++jumps > 4) {
        say("jumped back to a setjmp that answered 0\n");
        quit(100);
    }
    longjmp(env, v);
}
__attribute__((noinline)) static void second(int v) { third(v); }
__attribute__((noinline)) static void first(int v) { second(v); }

/* The callee-saved registers: probe() sets known values and calls cstd_run(), which takes a
 * setjmp and calls smash(). smash() overwrites them (its prologue saves them, its epilogue never
 * runs) and jumps back, so only longjmp can have put them back. r7 is the frame pointer on thumb
 * and is left out. On a Cortex-M with floating-point registers s16-s31 are callee-saved too:
 * probe() gives them 2.0 to 17.0 and stores them before and after, and smash() sets them to 1.0. */
static int jumped;

#if defined(__thumb__) && defined(__ARM_FP)
#define FP_SET                                                                                    \
    "vmov.f32 s16, #2.0\n vmov.f32 s17, #3.0\n vmov.f32 s18, #4.0\n vmov.f32 s19, #5.0\n"         \
    "vmov.f32 s20, #6.0\n vmov.f32 s21, #7.0\n vmov.f32 s22, #8.0\n vmov.f32 s23, #9.0\n"         \
    "vmov.f32 s24, #10.0\n vmov.f32 s25, #11.0\n vmov.f32 s26, #12.0\n vmov.f32 s27, #13.0\n"     \
    "vmov.f32 s28, #14.0\n vmov.f32 s29, #15.0\n vmov.f32 s30, #16.0\n vmov.f32 s31, #17.0\n"     \
    "vstmia r2, {s16-s31}\n"
#define FP_SAVE "adds r2, #64\n vstmia r2, {s16-s31}\n"
#define FP_CLOBBERS , "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26", \
                      "s27", "s28", "s29", "s30", "s31"
#else
#define FP_SET ""
#define FP_SAVE ""
#define FP_CLOBBERS
#endif

__attribute__((noinline)) static void smash(void) {
#if defined(__thumb__)
    __asm__ volatile("movs r4, #1\n movs r5, #1\n movs r6, #1\n"
                     "mov r8, r4\n mov r9, r4\n mov r10, r4\n mov r11, r4\n"
                     ::: "r4", "r5", "r6", "r8", "r9", "r10", "r11");
#if defined(__ARM_FP)
    __asm__ volatile("vmov.f32 s16, #1.0\n vmov.f32 s17, s16\n vmov.f32 s18, s16\n vmov.f32 s19, s16\n"
                     "vmov.f32 s20, s16\n vmov.f32 s21, s16\n vmov.f32 s22, s16\n vmov.f32 s23, s16\n"
                     "vmov.f32 s24, s16\n vmov.f32 s25, s16\n vmov.f32 s26, s16\n vmov.f32 s27, s16\n"
                     "vmov.f32 s28, s16\n vmov.f32 s29, s16\n vmov.f32 s30, s16\n vmov.f32 s31, s16\n"
                     ::: "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23", "s24", "s25", "s26",
                         "s27", "s28", "s29", "s30", "s31");
#endif
#else
    __asm__ volatile("li s1, 1\n li s2, 1\n li s3, 1\n li s4, 1\n li s5, 1\n li s6, 1\n"
                     "li s7, 1\n li s8, 1\n li s9, 1\n li s10, 1\n li s11, 1\n"
                     ::: "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11");
#endif
    longjmp(env, 5);
}

__attribute__((noinline)) void cstd_run(void) {
    int r = setjmp(env);
    if (r == 0) smash();
    jumped = r;
}

static long regs[11];
static unsigned long fregs[32]; /* s16-s31 as set, then s16-s31 after the jump */

__attribute__((noinline)) static void probe(void) {
#if defined(__thumb__)
    register long *p __asm__("r0") = regs;
    register unsigned long *q __asm__("r2") = fregs;
    __asm__ volatile(
        "push {r0, r2}\n"
        FP_SET
        "movs r4, #4\n movs r5, #5\n movs r6, #6\n"
        "movs r1, #8\n mov r8, r1\n movs r1, #9\n mov r9, r1\n"
        "movs r1, #10\n mov r10, r1\n movs r1, #11\n mov r11, r1\n"
        "bl cstd_run\n"
        "pop {r0, r2}\n"
        FP_SAVE
        "str r4, [r0, #0]\n str r5, [r0, #4]\n str r6, [r0, #8]\n"
        "mov r1, r8\n str r1, [r0, #12]\n mov r1, r9\n str r1, [r0, #16]\n"
        "mov r1, r10\n str r1, [r0, #20]\n mov r1, r11\n str r1, [r0, #24]\n"
        : "+r"(p), "+r"(q)
        :
        : "r1", "r3", "r4", "r5", "r6", "r8", "r9", "r10", "r11", "r12", "lr", "memory", "cc"
          FP_CLOBBERS);
#else
    register long *p __asm__("a0") = regs;
    __asm__ volatile(
        "addi sp, sp, -16\n sw a0, 0(sp)\n"
        "li s1, 101\n li s2, 102\n li s3, 103\n li s4, 104\n li s5, 105\n li s6, 106\n"
        "li s7, 107\n li s8, 108\n li s9, 109\n li s10, 110\n li s11, 111\n"
        "call cstd_run\n"
        "lw a0, 0(sp)\n addi sp, sp, 16\n"
        "sw s1, 0(a0)\n sw s2, 4(a0)\n sw s3, 8(a0)\n sw s4, 12(a0)\n sw s5, 16(a0)\n"
        "sw s6, 20(a0)\n sw s7, 24(a0)\n sw s8, 28(a0)\n sw s9, 32(a0)\n sw s10, 36(a0)\n"
        "sw s11, 40(a0)\n"
        : "+r"(p)
        :
        : "ra", "t0", "t1", "t2", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "t3", "t4", "t5", "t6",
          "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "memory");
#endif
}

static char *first_here;
static int same = 1;
static int last;

__attribute__((noinline)) static void dive(int i) {
    volatile char here = 0;
    if (i == 0) first_here = (char *)&here;
    else if ((char *)&here != first_here) same = 0;
    last = i + 1;
    longjmp(env, i + 1);
}

int main(void) {
#if defined(__thumb__) && defined(__ARM_FP)
    /* CPACR: full access to coprocessors 10 and 11, the floating-point unit, before any uses it. */
    *(volatile unsigned *)0xE000ED88 |= 0xFu << 20;
    __asm__ volatile("dsb\n isb" ::: "memory");
#endif
    volatile int r = setjmp(env);
    if (r == 0) first(42);
    check("value", r, 42);

    r = setjmp(env);
    if (r == 0) first(0);
    check("zero", r, 1);

    probe();
    check("jumped", jumped, 5);
#if defined(__thumb__)
    static const long want[7] = {4, 5, 6, 8, 9, 10, 11};
    for (int i = 0; i < 7; i++) check("register", regs[i], want[i]);
#if defined(__ARM_FP)
    check("s16 as set", (long)fregs[0], 0x40000000); /* 2.0f */
    for (int i = 0; i < 16; i++) check("fp register", (long)fregs[16 + i], (long)fregs[i]);
#endif
#else
    for (int i = 0; i < 11; i++) check("register", regs[i], 101 + i);
#endif

    for (volatile int i = 0; i < 10000; i++)
        if (setjmp(env) == 0) dive(i);
    check("stack", same, 1);
    check("loops", last, 10000);

    say(failures ? "setjmp: FAILED\n" : "setjmp: ok\n");
    quit(failures);
    return 0;
}
