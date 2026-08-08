[BITS 16]

global _start
global jump_to_kernel
extern stage2_main
extern g_drive

%define KERNEL_ENTRY 0x9000
%define STAGE2_BASE  0x5000
%define CODE_SEL     0x08
%define DATA_SEL     0x10

section .text.entry

_start:
    cli

    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov sp, 0xFFFE

    sti

    mov [g_drive], dl

    push dx
    mov si, message
    call puts
    pop dx

    call stage2_main


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

    cli

    lgdt [gdt_descriptor]

    mov eax, cr0
    or  eax, 0x01
    mov cr0, eax

    jmp CODE_SEL:(STAGE2_BASE + protected_mode_entry)

[BITS 32]
protected_mode_entry:

    mov ax, DATA_SEL
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9F000

    mov word [0xB8000], 0x0F50

    mov eax, KERNEL_ENTRY
    jmp eax


[BITS 16]
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

align 8
gdt_start:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd STAGE2_BASE + gdt_start
