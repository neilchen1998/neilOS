
; Stage 1 Bootloader

org 0x7C00
bits 16


%define ENDL 0x0D, 0x0A

%define STAGE2_SEGMENT      0x0800
%define STAGE2_OFFSET       0x0000

%define STAGE2_FIRST_LBA    2864
%define STAGE2_SECTORS      16

; FAT12 Boot Sector Header (BIOS Parameter Block)
jmp short start          ; jumps over BPB
nop                      ; 8 bytes

; BIOS Parameter Block
OEMLabel            db "MSWIN4.1"   ; OEM Identifier
BytesPerSector      dw 512          ; Bytes per sector
SectorsPerCluster   db 1            ; Sectors per cluster
ReservedSectors     dw 1            ; Reserved sectors (boot record)
NumberofFATs        db 2            ; Number of FAT tables
RootEntries         dw 224          ; Number of root directory entries
TotalSectors        dw 2880         ; Total sectors (1.44MB)
Media               db 0xf0         ; Media descriptor (fixed/floppy)
SectorsPerFAT       dw 9            ; Sectors per FAT table
SectorsPerTrack     dw 18           ; Sectors per track
HeadsPerCylinder    dw 2            ; Number of magnetic heads
HiddenSectors       dd 0            ; Hidden sectors
TotalSectorsBig     dd 0            ; Large sector count (unused)
DriveNumber         db 0            ; BIOS drive number (set by BIOS)
CurrentFlags        db 0            ; Reserved/Flags
BootSignature       db 0x29         ; Extended boot signature
VolumeID            dd 0x1337c0de   ; Volume serial number
VolumeLabel         db "NEIL OS    "; Volume label (11 bytes)
FileSystem          db "FAT12   "   ; File system type (8 bytes)


; Bootloader code begins here
start:

    cli

    xor ax, ax
    mov ds, ax
    mov es, ax

    ; Set up stack
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [DriveNumber], dl

    mov si, msg_loading
    call puts

    ; Load Stage 2
    mov ax, STAGE2_FIRST_LBA
    call lba_to_chs
    mov ax, STAGE2_SEGMENT
    mov es, ax
    xor bx, bx

    mov ah, 0x2
    mov al, STAGE2_SECTORS
    mov dl, [DriveNumber]

    int 0x13
    jc disk_error

    jmp STAGE2_SEGMENT:STAGE2_OFFSET

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
;   sector     = (LBA % SectorsPerTrack) + 1
;   head       = (LBA / SectorsPerTrack) % HeadsPerCylinder
;   cylinder   = (LBA / SectorsPerTrack) / HeadsPerCylinder
lba_to_chs:

    xor dx, dx
    div word [SectorsPerTrack]    ; AX = (LBA / SectorsPerTrack), DX = (LBA % SectorsPerTrack)

    inc dl                          ; DX = (LBA % SectorsPerTrack) + 1
    mov cl, dl                      ; CL = DL = sector

    xor dx, dx                      ; DX = 0
    div word [HeadsPerCylinder]                ; AX = (LBA / SectorsPerTrack) / HeadsPerCylinder = cylinder, DX = head

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

    mov byte [RetryCount], 3

.retry:

    push ax                  ; saves the LBA
    call lba_to_chs          ; converts AX to CH, CL, DH
    pop ax                   ; restores the LBA for future retries

    mov dl, [DriveNumber]   ; loads BIOS drive number

    mov ah, 02h              ; read sectors
    mov al, 1                ; sets the number of sectors
    stc                      ; sets carry flag (CF)
    int 13h                  ; calls BIOS disk services
    jnc .success             ; BIOS uses CF to report if the result is success
                             ; only jumps to the next line if failure

    call disk_reset

    dec byte [RetryCount]
    jnz .retry

    popa
    jmp disk_error

.success:

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

    mov ah, 0           ; resets disk system
    mov dl, [BootDrive] ; loads BootDrive
    stc
    int 13h

    popa
    ret

disk_error:

    mov si, msg_error
    call puts

.wait:

    xor ah, ah
    int 16h

    jmp 0xFFFF:0x0

; Data
BootDrive      db 0
RetryCount     db 0

msg_loading    db "Loading Stage 2...", ENDL,0
msg_done       db "OK",                ENDL,0
msg_error      db "Disk Read Error",   ENDL,0

times 510-($-$$) db 0
dw 0AA55h
