#include "test_assert.h"

#include "drivers/video/vga.h"

void test_assert(const char* msg, bool condition)
{
    if (!condition)
    {
        vga_set_color(VGA_RED, VGA_BLACK);
        fvga_write("ASSERT FAILED: ");
        fvga_write(msg);
        fvga_write("\n");
        vga_set_color(VGA_WHITE, VGA_BLACK);
    }
}
