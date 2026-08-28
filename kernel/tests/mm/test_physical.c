#include "test_physical.h"

#include <stddef.h>
#include <stdint.h>

#include "drivers/video/vga.h"
#include "mm/physical.h"
#include "test_output.h"

void physical_test(void)
{
    uint32_t a = physical_alloc_page();
    uint32_t b = physical_alloc_page();
    uint32_t c = physical_alloc_page();

    fvga_write("physical test:\n");

    fvga_write("  a = %x\n", a);
    fvga_write("  b = %x\n", b);
    fvga_write("  c = %x\n", c);

    if (a == 0 || b == 0 || c == 0)
    {
        test_fail("  FAIL: allocation returned 0\n");
        return;
    }

    if ((a & 0xFFFu) != 0 || (b & 0xFFFu) != 0 || (c & 0xFFFu) != 0)
    {
        test_fail("  FAIL: page is not aligned\n");
        return;
    }

    if (a == b || a == c || b == c)
    {
        test_fail("  FAIL: duplicate page\n");
        return;
    }

    if (a < 0x00200000u || b < 0x00200000u || c < 0x00200000u)
    {
        test_fail("  FAIL: page overlaps heap\n");
        return;
    }

    physical_free_page(b);

    uint32_t d = physical_alloc_page();

    fvga_write("  d = %x\n", d);

    if (d != b)
    {
        test_fail("  FAIL: freed page was not reused\n");
        return;
    }

    test_pass("PHYSICAL TEST  PASS\n");
}
