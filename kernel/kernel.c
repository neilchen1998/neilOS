#include "drivers/video/vga.h"

void kmain(void)
{
    terminal_init();

    terminal_write("Hello, from neilOS!\n");

    for (;;)
    {
        __asm__ volatile ("hlt");
    }
}
