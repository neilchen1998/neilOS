[BITS 32]

section .text

extern interrupt_handler

isr_common:

    pusha

    push esp
    call interrupt_handler
    add esp, 4

    popa
    add esp, 8

    iretd

; ISR_NOERROR:
;   Defines an ISR entry point for interrupts that do not automatically push an error code onto the stack.
;   This is intended for interrupts/execptions where the CPU does not automatically push an error code.
;
; %1:
;   Interrupt vector number.
%macro ISR_NOERROR 1
global isr%1
isr%1:

    push dword 0    ; pass in fake error code
    push dword %1   ; pushes the interrupt vector number onto the stack
    jmp  isr_common
%endmacro

; ISR_ERROR:
;   Defines an ISR entry point for interrupts.
;
; %1:
;   Interrupt vector number.
%macro ISR_ERROR 1
global isr%1
isr%1:

    push dword %1   ; pushes the interrupt vector number onto the stack
    jmp  isr_common
%endmacro

; CPU exceptions 0-31
ISR_NOERROR 0       ; Divide Error
ISR_NOERROR 1       ; Debug
ISR_NOERROR 2       ; NMI
ISR_NOERROR 3       ; Breakpoint
ISR_NOERROR 4       ; Overflow
ISR_NOERROR 5       ; BOUND Range Exceeded
ISR_NOERROR 6       ; Invalid Opcode
ISR_NOERROR 7       ; Device Not Available

ISR_ERROR   8       ; Double Fault

ISR_NOERROR 9       ; Coprocessor Segment Overrun (legacy)
ISR_ERROR   10      ; Invalid TSS
ISR_ERROR   11      ; Segment Not Present
ISR_ERROR   12      ; Stack-Segment Fault
ISR_ERROR   13      ; General Protection Fault
ISR_ERROR   14      ; Page Fault

ISR_NOERROR 15      ; Reserved
ISR_NOERROR 16      ; x87 Floating-Point Exception
ISR_ERROR   17      ; Alignment Check
ISR_NOERROR 18      ; Machine Check
ISR_NOERROR 19      ; SIMD Floating-Point Exception
ISR_NOERROR 20      ; Virtualization Exception
ISR_ERROR   21      ; Control Protection Exception

ISR_NOERROR 22      ; Reserved
ISR_NOERROR 23      ; Reserved
ISR_NOERROR 24      ; Reserved
ISR_NOERROR 25      ; Reserved
ISR_NOERROR 26      ; Reserved
ISR_NOERROR 27      ; Reserved
ISR_NOERROR 28      ; Hypervisor Injection Exception
ISR_NOERROR 29      ; VMM Communication Exception
ISR_ERROR   30      ; Security Exception
ISR_NOERROR 31      ; Reserved

%assign i 32
%rep 16
    ISR_NOERROR i
%assign i i + 1
%endrep

%assign i 48
%rep 208
    ISR_NOERROR i
%assign i i + 1
%endrep
