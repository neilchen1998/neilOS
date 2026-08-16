#include <stdint.h>

#include "arch/x86/idt.h"
#include "arch/x86/pic.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/timer/pit.h"
#include "drivers/video/vga.h"
#include "scheduler/scheduler.h"

static volatile uint32_t cntA = 0;
static volatile uint32_t cntB = 0;

#ifdef DEBUG
static void task_a(void)
{
    for (;;)
    {
        cntA++;
        task_yield();
    }
}

static void task_b(void)
{
    for (;;)
    {
        cntB++;
        task_yield();
    }
}

static void taskA(void)
{
    for (;;)
    {
        fterminal_write("A: %i\n", cntA);
        task_yield();
    }
}

static void taskB(void)
{
    for (;;)
    {
        fterminal_write("B: %i\n", cntB);
        task_yield();
    }
}
#endif

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

    terminal_write("Interrupts enabled!\n");

#ifdef DEBUG
    task_create(taskA);
    task_create(taskB);
    task_create(task_a);
    task_create(task_b);

    terminal_write("Tasks created!\n");
#endif

    __asm__ volatile ("sti");

    for (;;)
    {
        __asm__ volatile ("hlt");
    }
}
