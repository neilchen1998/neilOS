#include "test_assert.h"

#include "drivers/video/vga.h"
#include "stddef.h"

static void expect_fail(const char* msg)
{
    vga_set_color(VGA_RED, VGA_BLACK);
    fvga_write("ASSERT FAILED: ");
    fvga_write(msg);
    fvga_write("\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
}

void EXPECT_TRUE(const char* msg, bool condition)
{
    if (!condition)
    {
        expect_fail(msg);
    }
}

void EXPECT_FALSE(const char* msg, bool condition)
{
    if (condition)
    {
        expect_fail(msg);
    }
}

void EXPECT_EQ(const char* msg, int val1, int val2)
{
    if (val1 != val2)
    {
        expect_fail(msg);
    }
}

void EXPECT_NE(const char* msg, int val1, int val2)
{
    if (val1 == val2)
    {
        expect_fail(msg);
    }
}

void EXPECT_LT(const char* msg, int val1, int val2)
{
    if (val1 >= val2)
    {
        expect_fail(msg);
    }
}

void EXPECT_LE(const char* msg, int val1, int val2)
{
    if (val1 > val2)
    {
        expect_fail(msg);
    }
}

void EXPECT_GT(const char* msg, int val1, int val2)
{
    if (val1 <= val2)
    {
        expect_fail(msg);
    }
}

void EXPECT_GE(const char* msg, int val1, int val2)
{
    if (val1 < val2)
    {
        expect_fail(msg);
    }
}
