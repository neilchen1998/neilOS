[BITS 16]

global _start
extern stage2_main

section .text

_start:
    cli

    ; Set up stack
    xor ax, ax
    mov ss, ax
    mov sp, 0x7C00

    sti

    call stage2_main

hang:
    cli
    hlt
    jmp hang
