[BITS 16]

global _start
global jump_to_kernel
extern stage2_main

section .text.entry

_start:
    cli

    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    xor esp, esp
    mov sp, 0xFFFE

    sti

    push dx
    mov si, message
    call puts
    pop dx

    movzx eax, dl
    push  eax
    o32   call stage2_main
    add   esp, 4

hang:
    cli
    hlt
    jmp hang


; jump_to_kernel:
;   Jumps to the loaded kernel iamge at physical address 0x9000.
;
; Returns:
;   No return value.
;
; Preserves:
;   All general purpose registers.
jump_to_kernel:

    mov ax, ds          ; copies the current data segment into AX
    add ax, 0x0400      ; adds 0x400 paragraphs (0x4000 bytes) to AX
    push ax
    push word 0x0000    ; places the destination offset on the stack

    retf


puts:
    cld
    lodsb
    test al, al
    jz .done
    mov ah, 0Eh
    mov bh, 0
    int 10h
    jmp puts
.done:
    ret

message db "Stage 2 reached!",13,10,0
