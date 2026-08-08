[BITS 16]

global disk_get_params
global disk_read_sector

extern g_drive
extern g_sectors_per_track
extern g_num_heads

section .text

; disk_get_params:
;   Retrieves the BIOS geometry of the boot drive using INT 13h, AX = 8h.
;
; Parameters:
;   None
;
; Returns:
;   EAX = 0  on success.
;   EAX = -1 on failure.
;
; Clobbers:
;   EAX, ECX, EDX, DI
disk_get_params:

    ; Create a stack frame and preserves ESI & EDI
    push ebp
    mov  ebp, esp
    push esi
    push edi

    ; Load the boot drive number from g_drive
    movzx eax, byte [g_drive]
    mov   dl, al

    ; Call INT 13h
    xor di, di
    mov ah, 0x08
    int 0x13
    jc  .error

    ; Extract sectors per track
    and   cl, 0x3F
    movzx ax, cl
    mov   [g_sectors_per_track], ax

    ; Extract number per heads
    movzx ax, dh
    inc   ax
    mov   [g_num_heads], ax

    xor eax, eax
    jmp .exit

.error:

    mov eax, -1

.exit:

    pop edi
    pop esi
    mov esp, ebp
    pop ebp
    ret

; lba_to_chs:
;   32-bit LBA to CHS conversion routine required by BIOS disk services (INT 13h).
;
; Paramters:
;   EAX - Input 32-bit LBA address.
;
; Returns:
;   CH - Low 8 bits of cyclinder.
;   CL - Sector & high 2 bits of cyclinder.
;   DH - Head.
;
; Clobbers:
;   AX, CX, DX.
;
; Formula:
;   sector     = (LBA % SectorsPerTrack) + 1
;   head       = (LBA / SectorsPerTrack) % HeadsPerCylinder
;   cylinder   = (LBA / SectorsPerTrack) / HeadsPerCylinder
lba_to_chs:

    movzx ecx, word [g_sectors_per_track]
    xor   edx, edx
    div   ecx                       ; EAX = LBA / SectorsPerTrack
                                    ; EDX = LBA % SectorsPerTrack

    mov ebx, edx
    inc ebx                         ; EBX = (LBA % SectorsPerTrack) + 1 = sector

    movzx ecx, word [g_num_heads]
    xor edx, edx
    div ecx                         ; EAX = cyclinder
                                    ; EDX = head

    mov esi, eax                    ; ESI = cylinder
    mov edi, edx                    ; EDI = head

    ret


; disk_read_sector:
;   Reads one sector from disk using BIOS disk services (INT 13h).
;
; Paramters:
;   [ebp + 8]  - LBA address
;   [ebp + 12] - Destination segment
;   [ebp + 16] - Destination offset
;
; Returns:
;   EAX = 0  on success.
;   EAX = -1 on failure.
;
; Clobbers:
;   EAX, EBX, ECX, EDX, ESI, and EDI
disk_read_sector:

    push ebp
    mov  ebp, esp

    push ebx
    push esi
    push edi

    mov eax, [ebp + 8]  ; EAX = lba
    call lba_to_chs

    mov eax, esi
    mov ch, al      ; CH = cylinder low 8 bits

    shr eax, 2
    and al, 0xC0
    and bl, 0x3F
    or  al, bl
    mov cl, al      ; CL = sector | cylinder high bits

    mov eax, edi
    mov dh,  al     ; DL = head

    mov dl, byte [g_drive]

    mov ax, word [ebp + 12]  ; BX = buf_off
    mov es, ax

    mov bx, word [ebp + 16]    ; offset

    mov si, 3

.retry:

    mov ax, 0x0201
    int 0x13

    jnc .done

    xor ax, ax
    int 0x13

    dec si
    jnz .retry

.error:

    mov eax, -1
    jmp .exit

.done:

    xor eax, eax

.exit:

    pop edi
    pop esi
    pop ebx

    mov esp, ebp
    pop ebp

    ret
