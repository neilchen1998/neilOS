global isr0
global irq1

extern keyboard_handler

isr0:

    cli

.hang:

    hlt
    jmp .hang

irq1:

    pusha

    call keyboard_handler

    popa

    iretd
