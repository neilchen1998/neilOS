#include <stdint.h>

#include "arch/x86/idt.h"
#include "arch/x86/pic.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/timer/pit.h"
#include "drivers/video/vga.h"
#include "scheduler/scheduler.h"
#include "mm/kmalloc.h"

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
    kmalloc_init();

    terminal_write("Hello, from neilOS!\n");

#ifdef DEBUG
    int* a = kmalloc(64);
    if (a)
    {
        a[0] = 1;
        a[1] = 2;

        if ((a[0] + a[1]) == 3)
        {
            terminal_write("kmalloc: OKAY!\n");
        }
        else
        {
            terminal_write("kmalloc: failed!\n");
        }

        kfree(a);
    }
    else
    {
        terminal_write("ERROR: kmalloc failed!\n");
    }
#endif

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
