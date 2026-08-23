#include <stddef.h>
#include <stdint.h>

#include "arch/x86/idt.h"
#include "arch/x86/pic.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/timer/pit.h"
#include "drivers/video/vga.h"
#include "mm/kmalloc.h"
#include "mm/paging.h"
#include "mm/physical.h"
#include "scheduler/scheduler.h"

#ifdef KERNEL_TESTS
#include "tests/tests.h"
#endif

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

    fterminal_write("Hello, from neilOS!\n");

    physical_init();

#ifdef KERNEL_TESTS
    kernel_tests_run();
#endif

    fterminal_write("Physical memory initialized!\n");

    paging_test();

    uint32_t page_a = physical_alloc_page();
    uint32_t page_b = physical_alloc_page();
    uint32_t page_c = physical_alloc_page();

    fterminal_write("page_a = %x\n", page_a);
    fterminal_write("page_b = %x\n", page_b);
    fterminal_write("page_c = %x\n", page_c);

    physical_free_page(page_b);

    uint32_t page_d = physical_alloc_page();

    fterminal_write("page_d = %x\n", page_d);

    kmalloc_init();

#ifdef DEBUG
    int* a = kmalloc(64);
    if (a)
    {
        a[0] = 1;
        a[1] = 2;

        if ((a[0] + a[1]) == 3)
        {
            fterminal_write("kmalloc: OKAY!\n");
        }
        else
        {
            fterminal_write("kmalloc: failed!\n");
        }

        kfree(a);
    }
    else
    {
        fterminal_write("ERROR: kmalloc failed!\n");
    }
#endif

    idt_init();

    fterminal_write("IDT initialized!\n");

    pic_init();

    fterminal_write("PIC initialized!\n");

    pit_init(PIT_FREQUENCY);

    fterminal_write("PIT initialized!\n");

    keyboard_init();

    fterminal_write("Keyboard initialized!\n");

    scheduler_init();
    fterminal_write("Scheduler initialized!\n");

    fterminal_write("Interrupts enabled!\n");

    blockerID = task_create(blocker);
    task_create(waker);
    task_create(monitor);
    task_create(task_a);
    task_create(task_b);
    task_create(spawner);

    fterminal_write("Tasks created!\n");

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
