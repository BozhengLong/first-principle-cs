# Boot loader for mini-os
# This is a simple boot sector that loads the kernel and jumps to it

.code16                     # Start in 16-bit real mode
.globl start
start:
    cli                     # Disable interrupts
    cld                     # Clear direction flag

    # Set up segment registers
    xorw    %ax, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss

    # Enable A20 line (to access memory above 1MB)
    inb     $0x92, %al
    orb     $0x02, %al
    outb    %al, $0x92

    # Load GDT
    lgdt    gdtdesc

    # Switch to protected mode
    movl    %cr0, %eax
    orl     $0x1, %eax
    movl    %eax, %cr0

    # Jump to 32-bit code
    ljmp    $0x08, $start32

.code32                     # Now in 32-bit protected mode
start32:
    # Set up segment registers for protected mode
    movw    $0x10, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
    movw    $0, %ax
    movw    %ax, %fs
    movw    %ax, %gs

    # Set up stack
    movl    $start, %esp

    # Call kernel main
    call    main

    # If main returns, halt
spin:
    hlt
    jmp     spin

# GDT (Global Descriptor Table)
.p2align 2
gdt:
    .word   0, 0
    .byte   0, 0, 0, 0

    # Code segment
    .word   0xFFFF, 0x0000
    .byte   0x00, 0x9A, 0xCF, 0x00

    # Data segment
    .word   0xFFFF, 0x0000
    .byte   0x00, 0x92, 0xCF, 0x00

gdtdesc:
    .word   gdtdesc - gdt - 1
    .long   gdt
