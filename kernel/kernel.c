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

void kmain(void)
{
    terminal_init();

    fterminal_write("Hello, from neilOS!\n");

    physical_init();

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

    fterminal_write("Tasks created!\n");

    __asm__ volatile("sti");

#ifdef KERNEL_TESTS
    kernel_tests_run();
#endif

    for (;;)
    {
        __asm__ volatile("hlt");
    }
}
