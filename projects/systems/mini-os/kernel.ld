/* Linker script for mini-os kernel */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
  /* Kernel loads at 1MB (0x100000) in physical memory */
  . = 0x80100000;

  .text : AT(0x100000) {
    *(.text .stub .text.* .gnu.linkonce.t.*)
  }

  PROVIDE(etext = .);

  .rodata : {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  }

  /* Align data section */
  . = ALIGN(0x1000);
  PROVIDE(data = .);

  .data : {
    *(.data)
  }

  .bss : {
    *(.bss)
  }

  PROVIDE(end = .);

  /DISCARD/ : {
    *(.eh_frame .note.GNU-stack)
  }
}
