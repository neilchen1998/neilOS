#include "arch/x86/idt.h"
#include "arch/x86/pic.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/video/vga.h"

void kmain(void)
{
    terminal_init();

    terminal_write("Hello, from neilOS!\n");

    idt_init();

    terminal_write("IDT initialized!\n");

    pic_init();

    terminal_write("PIC initialized!\n");

    keyboard_init();

    terminal_write("Keyboard initialized!\n");

    __asm__ volatile ("sti");

    terminal_write("Interrupts enabled!\n");

    for (;;)
    {
        __asm__ volatile ("hlt");
    }
}
