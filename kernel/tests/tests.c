#include "tests.h"

#include "drivers/video/vga.h"
#include "mm/test_kmalloc.h"
#include "mm/test_paging.h"
#include "mm/test_physical.h"
#include "scheduler/test_scheduler.h"
#include "syscall/test_syscall.h"

void kernel_tests_run(void)
{
    fvga_write("\n=== KERNEL TESTS ===\n");

    kmalloc_test();
    paging_test();
    physical_test();
    scheduler_test();
    syscall_test();

    fvga_write("=== TESTS COMPLETE ===\n");
}
