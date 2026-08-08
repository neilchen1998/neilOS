ENTRY(_start)

SECTIONS
{
    . = 0x0000;

    .text :
    {
        KEEP(*(.text.entry))
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
    }

    .data :
    {
        *(.data*)
    }

    .bss :
    {
        *(.bss*)
    }
}
