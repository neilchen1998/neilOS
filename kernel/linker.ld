ENTRY(_start)

SECTIONS
{
    . = 0x9000;

    __kernel_start = .;

    .text : ALIGN(16)
    {
        *(.text.entry)
        *(.text*)
    }

    .rodata : ALIGN(16)
    {
        *(.rodata*)
    }

    .data : ALIGN(16)
    {
        *(.data*)
    }

    .bss : ALIGN(16)
    {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = .;
    }

    __kernel_end = .;

    ASSERT(. <= 0x98000, "kernel image + bss overlaps the kernel stack")

    __heap_start = 0x100000;
    __heap_end = __heap_start + 0x100000;
}
