org 0x7C00
bits 16


%define ENDL 0x0D, 0x0A


; FAT12 Boot Sector Header (BIOS Parameter Block)
jmp short start          ; Jump over BPB
nop

; OEM Identifier
db "MSWIN4.1"            ; 8 bytes

; BIOS Parameter Block
bytes_per_sector:       dw 512
sectors_per_cluster:    db 1
reserved_sectors:       dw 1
num_fats:               db 2
root_entries:           dw 224
total_sectors:          dw 2880
media:                  db 0xF0
sectors_per_fat:        dw 9
sectors_per_track:      dw 18
heads:                  dw 2
hidden_sectors:         dd 0
large_sectors:          dd 0

; Extended BPB
drive_number:           db 0
reserved:               db 0
boot_signature:         db 0x29
volume_id:              dd 0x12345678
volume_label:           db "NEIL OS    "
filesystem:             db "FAT12   "

start:
    ; Bootloader code begins here
    cli

    xor ax, ax
    mov ds, ax
    mov es, ax

    jmp main

; puts:
;   Prints a null-terminated string to the screen using BIOS teletype output.
;
; Paramters:
;   DS:SI - Pointer to the input string.
;
; Returns:
;   None.
;
; Clobbers:
;   AL, AH, BH are modified.
;
; Preserves:
;   SI, AX are saved on entry and restored before returning.
puts:
    push si
    push ax

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


; lba_to_chs:
;   16-bit LBA to CHS conversion routine required by BIOS disk services (INT 13h).
;
; Paramters:
;   AX - Input 16-bit LBA address.
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
;   sector     = (LBA % sectors_per_track) + 1
;   head       = (LBA / sectors_per_track) % heads
;   cylinder   = (LBA / sectors_per_track) / heads
lba_to_chs:
    xor dx, dx
    div word [sectors_per_track]    ; AX = (LBA / sectors_per_track), DX = (LBA % sectors_per_track)

    inc dl                          ; DX = (LBA % sectors_per_track) + 1
    mov cl, dl                      ; CL = DL = sector

    xor dx, dx                      ; DX = 0
    div word [heads]                ; AX = (LBA / sectors_per_track) / heads = cylinder, DX = head

    mov dh, dl                      ; DH = head
    mov ch, al                      ; cylinder low 8 bits

    shl ah, 6                       ; cylinder bits 8-9 -> bits 6-7
    and cl, 3Fh                     ; keep sector bits
    or  cl, ah

    ret


; disk_read:
;   Reads a single sector from disk using BIOS disk services (INT 13h).
;
; Paramters:
;   AX - Logical block address (LBA) of the sector to read.
;   ES:BX - Destination buffer for the sector.
;
; Returns:
;   Success:
;       - Sector data is stored at ES:BX.
;   Failure:
;       - Jumps to floppy_error.
;
; Preserves:
;   All general purpose registers.
disk_read:
    pusha

    mov byte [retry_cnt], 3

.retry:
    push ax                  ; saves the LBA
    call lba_to_chs          ; converts AX to CH, CL, DH
    pop ax                   ; restores the LBA for future retries

    mov dl, [drive_number]   ; loads BIOS drive number

    mov ah, 02h              ; read sectors
    mov al, 1                ; sets the number of sectors
    stack                    ; sets carry flag (CF)
    int 13h                  ; calls BIOS disk services
    jnc .done                ; BIOS uses CF to report if the result is success
                             ; only jumps to the next line if failure

    call disk_reset

    dec byte [retry_cnt]
    jnz .retry

    popa
    jmp floppy_error

.done:
    popa
    ret


; disk_reset:
;   Resets the BIOS disk system (INT 13h, AH=0h).
; Returns:
;   No return value.
;
; Preserves:
;   All general purpose registers.
disk_reset:

    pusha
    mov ah, 0   ; resets disk system
    stc
    int 13h

    popa
    ret

main:

    ; Set up data segments
    mov ax, 0
    mov ds, ax
    mov es, ax

    ; Set up stack
    mov ss, ax
    mov sp, 0x7C00

    ; Get the drive number from BIOS through DL
    mov [drive_number], dl

    mov ax, 1
    mov cl, 1
    mov bx, 0x7E00
    call disk_read

    ; Print msg
    mov si, msg_hello
    call puts

    cli
    hlt

floppy_error:
    mov si, msg_read_failed
    call puts
    jmp wait_key_and_reboot

wait_key_and_reboot:
    mov ah, 0
    int 16h             ; waits for keypress
    jmp 0FFFFh:0000h    ; jumps to the beginning of BIOS
    hlt

.halt:
    cli             ; disables interrupt
    hlt


msg_hello:       db 'Hello, world!', ENDL, 0
msg_read_failed: db 'Failed to read from disk!', ENDL, 0

retry_cnt:      db 0

times 510-($-$$) db 0
dw 0AA55h
