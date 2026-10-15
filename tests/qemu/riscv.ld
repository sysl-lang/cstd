/* QEMU virt's RAM, at 0x8000_0000; started with -bios none, so the image is all there is. */
ENTRY(_start)
MEMORY {
  RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 16M
}
SECTIONS {
  .text : { *(.text.start) *(.text*) *(.rodata*) *(.srodata*) } > RAM
  .data : { *(.data*) *(.sdata*) } > RAM
  .bss : { *(.bss*) *(.sbss*) *(COMMON) } > RAM
  _stack_top = ORIGIN(RAM) + LENGTH(RAM);
}
