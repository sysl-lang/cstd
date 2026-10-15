/* Reset for the RV32 case: a stack and main. */
    .section .text.start, "ax"
    .globl _start
_start:
    la sp, _stack_top
    call main
1:  j 1b
