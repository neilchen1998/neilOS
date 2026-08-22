#include <stddef.h>
#include <stdint.h>

#include "arch/x86/idt.h"
#include "arch/x86/pic.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/timer/pit.h"
#include "drivers/video/vga.h"
#include "mm/kmalloc.h"
#include "mm/physical.h"
#include "scheduler/scheduler.h"

static volatile uint32_t cntA = 0;
static volatile uint32_t cntB = 0;

static size_t blockerID = -1;

static void task_a(void)
{
    while (timer_ms() < 5000)
    {
        cntA++;
    }
}

static void task_b(void)
{
    while (timer_ms() < 5000)
    {
        cntB++;
    }
}

static void monitor(void)
{
    uint64_t next = 0;

    while (timer_ms() < 5000)
    {
        if (timer_ms() >= next)
        {
            fterminal_write("A: %i B:%i\n", cntA, cntB);
            next = timer_ms() + 500;
        }

        task_yield();
    }

    fterminal_write("Preemptive test done!\t A: %i, B:%i\n", cntA, cntB);
}

static void blocker(void)
{
    fterminal_write("blocker: sleeping...\n");
    task_block();
    fterminal_write("blocker: woken!\n");
    task_exit();
}

static void waker(void)
{
    while (timer_ms() < 5000)
    {
        task_yield();
    }

    fterminal_write("waker: unblock %i -> %i\n", blockerID, task_unblock(blockerID));
    task_exit();
}

static void short_lived(void)
{
    task_exit();
}

static void spawner(void)
{
    uint32_t fails = 0;

    for (uint32_t i = 0; i < 1000; ++i)
    {
        if (task_create(short_lived) < 0)
        {
            ++fails;
        }

        task_yield();
    }

    fterminal_write("spawner done: %i failure(s).\n", fails);

    task_exit();
}

void kmain(void)
{
    terminal_init();
    kmalloc_init();
    physical_init();

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

    blockerID = task_create(blocker);
    task_create(waker);
    task_create(monitor);
    task_create(task_a);
    task_create(task_b);
    task_create(spawner);

    terminal_write("Tasks created!\n");

    __asm__ volatile("sti");

    while (timer_ms() < 5000)
    {
        task_yield();
    }

    for (;;)
    {
        __asm__ volatile("hlt");
    }
}
