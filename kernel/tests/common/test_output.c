#include "test_output.h"

#include <stdarg.h>

#include "drivers/video/vga.h"

void test_pass(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    vga_set_color(VGA_GREEN, VGA_BLACK);
    fvga_write(format, args);
    vga_set_color(VGA_WHITE, VGA_BLACK);

    va_end(args);
}

void test_fail(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    vga_set_color(VGA_RED, VGA_BLACK);
    fvga_write(format, args);
    vga_set_color(VGA_WHITE, VGA_BLACK);

    va_end(args);
}
