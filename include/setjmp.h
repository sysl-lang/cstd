/* <setjmp.h> from sh.sysl.cstd: setjmp, longjmp, sigsetjmp, siglongjmp. Defined in
 * sh/sysl/cstd/setjmp.sysl, whose table gives each processor's layout.
 *
 * A jmp_buf is the registers a callee must preserve, then 128 bytes for a signal mask, as musl
 * lays it out; sigjmp_buf is the same type, so siglongjmp is longjmp under a second name.
 * sigsetjmp with a non-zero savemask reads and restores the mask through sigprocmask, which the
 * platform supplies. */
#ifndef CSTD_SETJMP_H
#define CSTD_SETJMP_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__aarch64__)
/* x19-x30, sp, d8-d15, and two words sigsetjmp keeps. */
#define __CSTD_JMP_WORDS 24
#elif defined(__thumb__)
#if defined(__ARM_FP)
#error "cstd's setjmp does not save s16-s31, which this floating-point build keeps across calls"
#endif
/* r4-r11, sp, lr, the place s16-s31 will take, and two words sigsetjmp keeps. */
#define __CSTD_JMP_WORDS 14
#elif defined(__riscv) && __riscv_xlen == 32 && !defined(__riscv_flen)
/* ra, sp, s0-s11, and two words sigsetjmp keeps. */
#define __CSTD_JMP_WORDS 8
#else
#error "cstd's setjmp is written for aarch64, thumb without a floating-point unit, and riscv32 without F"
#endif

typedef struct __cstd_jmp_buf {
    unsigned long long __regs[__CSTD_JMP_WORDS];
    unsigned long long __mask[16];
} jmp_buf[1];

typedef jmp_buf sigjmp_buf;

/* returns_twice is said outright, so a compiler told -fno-builtin still keeps what a second return
 * needs; noreturn is spelled so that C++ reads it too. */
__attribute__((__returns_twice__)) int setjmp(jmp_buf env);
__attribute__((__noreturn__)) void longjmp(jmp_buf env, int val);
__attribute__((__returns_twice__)) int sigsetjmp(sigjmp_buf env, int savemask);
__attribute__((__noreturn__)) void siglongjmp(sigjmp_buf env, int val);

#ifdef __cplusplus
}
#endif

#endif
