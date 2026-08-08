bits 32

%define COM1     0x03F8     ; I/O port address of the first PC serial port
%define DATA_SEL 0x10       ; data selector for GDT

section .text

global _start

extern kmain

_start:

    cli

    ; Establish memory/data env
    mov ax, DATA_SEL
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Establish stack
    mov esp, 0x9E000

    cld

    call serial_init

    mov esi, msg_kernel
    call serial_puts

    ; Run the kernel
    call kmain

.hang:

    cli
    hlt
    jmp .hang


; serial_init:
;   Initializes COM1 for serial communication.
;   Disables UART interrupts, sets the baud rate to 9600,
;   configures 8 data bits, enables and clears the UART FIFOs,
;   sets the receive FIFO trigger level to 14 bytes, and
;   enables the DTR, RTS, and UART interrupt output signals.
serial_init:

    ; Disable interrupts
    mov dx, COM1 + 1    ; COM1 + 1 is the interrupt enable register (IER)
    mov al, 0x00
    out dx, al          ; sends data from CPU to an I/O port

    ; Prepare to set the baud rate divisor
    mov dx, COM1 + 3    ; COM1 + 3 is the line control register (LCR)
    mov al, 0x80        ; sets the divisor latch access bit (DLAB) to 1
    out dx, al

    ; Set baud rate divisor
    ; The desired baud rate = 1,843,200 / (16 * baud rate divisor)
    mov dx, COM1 + 0    ; COM1 + 0 is the divisor latch low byte
    mov al, 0x0C        ; sets the divisor to 12
    out dx, al

    mov dx, COM1 + 1    ; COM1 + 1 is the divisor latch high byte
    mov al, 0x00
    out dx, al

    ; Configure line protocol
    mov dx, COM1 + 3
    mov al, 0b11        ; sets the data size to 8 bits per character
    out dx, al

    ; Set FIFO control register (FCR)
    mov dx, COM1 + 2    ; COM1 + 3 is the FIFO control register (FCR)
    mov al, 0xC7        ; Bit 0 = 1: enables the UART FIFOs.
                        ; Bit 1 = 1: clears the RX FIFO.
                        ; Bit 2 = 1: clears the TX FIFO.
                        ; Bits 6–7 = 11: sets the receive FIFO trigger level to 14 bytes
    out dx, al

    ; Set modem control register (MCR)
    mov dx, COM1 + 4    ; COM1 + 4 is the MCR
    mov al, 0x0B        ; Bit 0 = 1: DTR (data terminal ready)
                        ; Bit 1 = 1: request to send
                        ; Bit 3 = 1: enables UART's interrupt output on typical PC serial ports
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
