org 0x7C00
bits 16


%define ENDL 0x0D, 0x0A


times 510-($-$$) db 0
dw 0AA55h
