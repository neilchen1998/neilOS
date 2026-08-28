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
    vga_init();

    fvga_write("Hello, from neilOS!\n");

    physical_init();

    fvga_write("Physical memory initialized!\n");

    kmalloc_init();

    idt_init();
    fvga_write("IDT initialized!\n");

    pic_init();
    fvga_write("PIC initialized!\n");

    pit_init(PIT_FREQUENCY);
    fvga_write("PIT initialized!\n");

    keyboard_init();
    fvga_write("Keyboard initialized!\n");

    scheduler_init();
    fvga_write("Scheduler initialized!\n");

    fvga_write("Interrupts enabled!\n");
    fvga_write("Tasks created!\n");

    paging_init();
    fvga_write("Paging created!\n");

    __asm__ volatile("sti");

#ifdef KERNEL_TESTS
    kernel_tests_run();
#endif

    for (;;)
    {
        __asm__ volatile("hlt");
    }
}
