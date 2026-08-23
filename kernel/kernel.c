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

    kmalloc_init();

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

    paging_init();
    fterminal_write("Paging created!\n");

    __asm__ volatile("sti");

#ifdef KERNEL_TESTS
    kernel_tests_run();
#endif

    for (;;)
    {
        __asm__ volatile("hlt");
    }
}
