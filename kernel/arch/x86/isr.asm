global isr0

isr0:

    cli


.hang:

    hlt
    jmp .hang
