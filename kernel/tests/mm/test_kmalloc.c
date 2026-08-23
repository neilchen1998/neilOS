#include "test_kmalloc.h"

#include <stddef.h>
#include <stdint.h>

#include "drivers/video/vga.h"
#include "mm/kmalloc.h"
#include "test_output.h"

void kmalloc_test(void)
{
    fterminal_write("kmalloc: starting tests...\n");

    /* Test 1: Basic allocation and write/read */
    int* a = kmalloc(64);

    if (!a)
    {
        fterminal_write("kmalloc: FAIL - basic allocation\n");
        return;
    }

    a[0] = 0x12345678;
    a[1] = 0xDEADBEEF;

    if (a[0] != 0x12345678 || a[1] != 0xDEADBEEF)
    {
        fterminal_write("kmalloc: FAIL - basic read/write\n");
        kfree(a);
        return;
    }

    test_pass("kmalloc: basic allocation OK\n");

    /* Test 2: Multiple allocations */
    int* b = kmalloc(128);
    int* c = kmalloc(256);

    if (!b || !c)
    {
        test_fail("kmalloc: FAIL - multiple allocations\n");

        if (b)
            kfree(b);
        if (c)
            kfree(c);

        kfree(a);
        return;
    }

    b[0] = 111;
    c[0] = 222;

    if (b[0] != 111 || c[0] != 222)
    {
        test_fail("kmalloc: FAIL - multiple allocation data\n        ");
        kfree(a);
        kfree(b);
        kfree(c);
        return;
    }

    test_pass("kmalloc: multiple allocations OK\n");

    /* Test 3: Verify blocks don't overlap */
    a[0] = 10;
    b[0] = 20;
    c[0] = 30;

    if (a[0] != 10 || b[0] != 20 || c[0] != 30)
    {
        test_fail("kmalloc: FAIL - allocation overlap\n");
        kfree(a);
        kfree(b);
        kfree(c);
        return;
    }

    test_pass("kmalloc: isolation OK\n");

    /* Test 4: Free and allocate again */
    kfree(b);

    int* d = kmalloc(128);

    if (!d)
    {
        test_fail("kmalloc: FAIL - allocation after free\n");
        kfree(a);
        kfree(c);
        return;
    }

    d[0] = 444;

    if (d[0] != 444)
    {
        test_fail("kmalloc: FAIL - reuse after free\n");
        kfree(a);
        kfree(c);
        kfree(d);
        return;
    }

    test_pass("kmalloc: free/reuse OK\n");

    /* Clean up */
    kfree(a);
    kfree(c);
    kfree(d);

    test_pass("kmalloc: ALL TESTS PASSED!\n");
}
