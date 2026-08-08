org 0
bits 16


%define ENDL 0x0D, 0x0A
%define COM1 0x03F8


start:

    cli

    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xFFFE

    cld

    call serial_init

    mov si, msg_kernel
    call serial_puts

    mov si, msg_kernel
    call puts

    sti

.hang

    cli
    hlt
    jmp .hang


puts:

    push si
    push ax

    cld

.loop:
    lodsb           ; loads one byte from DS:SI into AL
    or al, al       ; checks if the character is NULL
    jz .done        ; jumps to .done if the character is NULL

    mov ah, 0x0e    ; selects BIOS video function (teletype output)
    mov bh, 0
    int 0x10        ; triggers BIOS video interrupt

    jmp .loop       ; continues back to the loop

.done:
    ; Pop the registers in reverse order
    pop ax
    pop si
    ret

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

    push ax
    push dx

    cld

.wait:
    mov dx, COM1 + 5
    in al, dx
    test al, 0x20
    jz .wait

    pop dx
    pop ax

    mov dx, COM1
    out dx, al
    ret

serial_puts:

    push si
    push ax

.loop:
    lodsb
    or al, al
    jz .done

    call serial_putc
    jmp .loop

.done:
    pop ax
    pop si
    ret

msg_kernel    db "Kernel reached!", ENDL,0
