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

    ; Entry 1: kernel code segment
    ;
    ; Selector:    0x08
    ; Index:       1
    ; RPL:         0 (ring 0)
    ;
    ; Base:        0x00000000
    ; Limit:       0x000FFFFF (encoded)
    ; Granularity: 4 KiB
    ; Effective limit: 0xFFFFFFFF
    ;
    ; Present:     yes
    ; DPL:         0
    ; Type:        code, execute/read
    ; Conforming:  no
    ; Readable:    yes
    ;
    ; Size:        32-bit (D = 1)
    ; Granularity: 4 KiB (G = 1)
    ;
    ; This is a flat 32-bit ring-0 code segment:
    ; 0x00000000 -> 0xFFFFFFFF
    dq 0x00CF9A000000FFFF

    ; Entry 2: kernel data segment
    ;
    ; Selector:    0x10
    ; Index:       2
    ; RPL:         0 (ring 0)
    ;
    ; Base:        0x00000000
    ; Limit:       0x000FFFFF (encoded)
    ; Granularity: 4 KiB
    ; Effective limit: 0xFFFFFFFF
    ;
    ; Present:     yes
    ; DPL:         0
    ; Type:        data, read/write
    ; Expand-down: no
    ;
    ; Size:        32-bit
    ; Granularity: 4 KiB
    ;
    ; This is a flat 32-bit ring-0 data segment:
    ; 0x00000000 -> 0xFFFFFFFF
    dq 0x00CF92000000FFFF

    ; Entry 3: user code segment
    ;
    ; Selector:    0x1B
    ; Index:       3
    ; RPL:         3 (ring 3)
    ;
    ; Descriptor DPL: 3
    ;
    ; Base:        0x00000000
    ; Limit:       0x000FFFFF (encoded)
    ; Granularity: 4 KiB
    ; Effective limit: 0xFFFFFFFF
    ;
    ; Present:     yes
    ; DPL:         3
    ; Type:        code, execute/read
    ; Conforming:  no
    ; Readable:    yes
    ;
    ; Size:        32-bit
    ; Granularity: 4 KiB
    ;
    ; This is a flat 32-bit ring-3 code segment:
    ; 0x00000000 -> 0xFFFFFFFF
    dq 0x00CFFA000000FFFF

    ; Entry 4: user data segment
    ;
    ; Selector:    0x23
    ; Index:       4
    ; RPL:         3 (ring 3)
    ;
    ; Descriptor DPL: 3
    ;
    ; Base:        0x00000000
    ; Limit:       0x000FFFFF (encoded)
    ; Granularity: 4 KiB
    ; Effective limit: 0xFFFFFFFF
    ;
    ; Present:     yes
    ; DPL:         3
    ; Type:        data, read/write
    ; Expand-down: no
    ;
    ; Size:        32-bit
    ; Granularity: 4 KiB
    ;
    ; This is a flat 32-bit ring-3 data segment:
    ; 0x00000000 -> 0xFFFFFFFF
    dq 0x00CFF2000000FFFF

kernel_gdt_end:


; GDTR
;
; Limit: 0x27
;
; struct gdt_ptr {
;     uint16_t limit;
;     uint32_t base;
; };
kernel_gdtr:

    dw kernel_gdt_end - kernel_gdt - 1  ; CPU interprets that as allowing offsets 0 through 39, which covers 40 bytes
    dd kernel_gdt
