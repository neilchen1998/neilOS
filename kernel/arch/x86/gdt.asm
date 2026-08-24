[BITS 32]

section .text

global gdt_flush
global kernel_gdtr

; gdt_flush
;
; void gdt_flush(struct gdt_ptr *gdtr)
;
; [ESP + 4] = address of gdtr
gdt_flush:

    ; Read the first argument at [ESP + 4] (the address of gdtr) and store it in EAX
    mov eax, [esp + 4]

    ; Load the kernel GDT (Global Descriptor Table)
    lgdt [eax]

    ; Kernel data selector = 0x10
    mov ax, 0x10

    mov ds, ax  ; DS: data segment
    mov es, ax  ; ES: extra segment
    mov fs, ax
    mov gs, ax
    mov ss, ax  ; SS: stack segment

    ; Kernel code selector = 0x08
    ; NOTE: mov cs, ax is invalid
    ; Use far jump to reload CS
    jmp 0x08:.reload_cs

.reload_cs:

    ret


section .rodata

align 8 ; ensures GDT starts at an address aligned to an 8-byte boundary

kernel_gdt:

    ; Entry 0: null
    dq 0x0000000000000000

    ; Entry 1: kernel code
    ; Selector = 0x08
    ;
    ; Base:        0x00000000
    ; Limit:       0xFFFFFFFF effective (0xFFFFF × 0x1000 + 0xFFF)
    ; Privilege:   Ring 0 (DPL = 0)
    ; Present:     yes
    ; Type:        executable/readable code
    ; Size:        32-bit
    ; Granularity: 1 (4 KiB)
    dq 0x00CF9A000000FFFF

    ; Entry 2: kernel data
    ;
    ; Selector = 0x10
    dq 0x00CF92000000FFFF

    ; Entry 3: user code
    ;
    ; Selector = 0x1B with RPL=3
    dq 0x00CFFA000000FFFF

    ; Entry 4: user data
    ;
    ; Selector = 0x23 with RPL=3
    dq 0x00CFF2000000FFFF

kernel_gdt_end:


; GDTR
;
; struct gdt_ptr {
;     uint16_t limit;
;     uint32_t base;
; };
kernel_gdtr:

    dw kernel_gdt_end - kernel_gdt - 1
    dd kernel_gdt
