/* Reset for the Cortex-M cases: the vector table, then a stack and main. QEMU starts at the ELF
 * entry, so _start sets sp itself. */
    .syntax unified
    .section .vectors, "a"
    .word _stack_top
    .word _start
    .rept 14
    .word hang
    .endr

    .text
    .thumb_func
    .globl _start
_start:
    ldr r0, =_stack_top
    mov sp, r0
    bl main
    .thumb_func
hang:
    b hang
    .ltorg
