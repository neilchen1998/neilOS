#include "tests.h"

// #include "mm/test_kmalloc.h"
// #include "mm/test_paging.h"
#include "drivers/video/vga.h"
#include "mm/test_physical.h"
// #include "scheduler/test_scheduler.h"

void kernel_tests_run(void)
{
    fterminal_write("\n=== KERNEL TESTS ===\n");

    physical_test();
    // paging_test();
    // kmalloc_test();
    // scheduler_test();

    fterminal_write("=== TESTS COMPLETE ===\n");
}
