bits 32

%define COM1 0x03F8

section .text

global _start

extern kmain

_start:
    cli

    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0xFFFE

    cld

    call serial_init

    mov esi, msg_kernel
    call serial_puts

    call kmain

.hang:
    cli
    hlt
    jmp .hang


serial_init:
    mov dx, COM1 + 1
    mov al, 0x00
    out dx, al

    mov dx, COM1 + 3
    mov al, 0x80
    out dx, al

    mov dx, COM1 + 0
    mov al, 0x03
    out dx, al

    mov dx, COM1 + 1
    mov al, 0x00
    out dx, al

    mov dx, COM1 + 3
    mov al, 0x03
    out dx, al

    mov dx, COM1 + 2
    mov al, 0xC7
    out dx, al

    mov dx, COM1 + 4
    mov al, 0x0B
    out dx, al

    ret


serial_putc:

    push eax
    push edx

.wait:
    mov dx, COM1 + 5
    in al, dx
    test al, 0x20
    jz .wait

    pop edx
    pop eax

    mov dx, COM1
    out dx, al

    ret


serial_puts:

    push esi
    push eax

.loop:
    lodsb
    test al, al
    jz .done

    call serial_putc
    jmp .loop

.done:
    pop eax
    pop esi
    ret


section .rodata

msg_kernel:
    db "Kernel reached!", 0x0D, 0x0A
