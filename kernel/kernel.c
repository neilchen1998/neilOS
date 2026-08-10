#include "arch/x86/idt.h"
#include "drivers/video/vga.h"

void kmain(void)
{
    terminal_init();

    terminal_write("Hello, from neilOS!\n");

    idt_init();

    terminal_write("IDT initialized!\n");

    for (;;)
    {
        __asm__ volatile ("hlt");
    }
}
