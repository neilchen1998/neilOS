[BITS 16]

global _start
extern stage2_main

section .text

_start:
    cli

    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xFFFE

    sti

    mov si, message
    call puts

    call stage2_main

hang:
    cli
    hlt
    jmp hang

puts:
    lodsb
    test al, al
    jz .done
    mov ah, 0Eh
    int 10h
    jmp puts
.done:
    ret

message db "Stage 2 reached!",13,10,0
