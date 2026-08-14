#include "arch/x86/idt.h"
#include "arch/x86/pic.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/timer/pit.h"
#include "drivers/video/vga.h"
#include "scheduler/scheduler.h"

static void taskA(void)
{
    for (;;)
    {
        terminal_write("A");
    }
}

static void taskB(void)
{
    for (;;)
    {
        terminal_write("B");
    }
}

void kmain(void)
{
    terminal_init();

    terminal_write("Hello, from neilOS!\n");

    idt_init();

    terminal_write("IDT initialized!\n");

    pic_init();

    terminal_write("PIC initialized!\n");

    pit_init(PIT_FREQUENCY);

    terminal_write("PIT initialized!\n");

    keyboard_init();

    terminal_write("Keyboard initialized!\n");

    scheduler_init();
    terminal_write("Scheduler initialized!\n");

    __asm__ volatile ("sti");

    terminal_write("Interrupts enabled!\n");

    task_create(taskA);
    task_create(taskB);

    for (;;)
    {
        __asm__ volatile ("hlt");
    }
}
