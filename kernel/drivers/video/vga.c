#include "vga.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/x86/io.h"

// VGA dimension
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define TERMINAL_LINES 1000

#define VGA_MEMORY ((volatile uint16_t*)0xB8000)

#define VGA_AT(row, col) VGA_MEMORY[(row) * VGA_WIDTH + (col)]

#define VGA_COLOR(fg, bg) ((uint8_t)(fg) | ((uint8_t)(bg) << 4))
#define VGA_ENTRY(ch, fg, bg) ((uint16_t)(ch) | (uint16_t)VGA_COLOR(fg, bg) << 8)
#define VGA_CLEAR VGA_ENTRY(' ', VGA_BLACK, VGA_BLACK)

static uint16_t terminalBuffer[TERMINAL_LINES][VGA_WIDTH];

static size_t cursorX = 0;
static size_t cursorY = 0;
static size_t viewportY = 0;
static bool renderPending = 0;
static uint8_t vgaColor = VGA_COLOR(VGA_WHITE, VGA_BLACK);

static void vga_render(void)
{
    for (size_t screenY = 0; screenY < VGA_HEIGHT; ++screenY)
    {
        size_t bufferY = viewportY + screenY;

        for (size_t x = 0; x < VGA_WIDTH; ++x)
        {
            VGA_MEMORY[screenY * VGA_WIDTH + x] = (bufferY < TERMINAL_LINES) ? terminalBuffer[bufferY][x] : VGA_CLEAR;
        }
    }
}

static void vga_follow_cursor(void)
{
    if (cursorY >= viewportY + VGA_HEIGHT)
    {
        viewportY = cursorY - VGA_HEIGHT + 1;
        renderPending = true;
    }
}

static void vga_write_cell(size_t bufferY, size_t x, uint16_t entry)
{
    if (!renderPending && bufferY >= viewportY && bufferY < viewportY + VGA_HEIGHT)
    {
        VGA_AT(bufferY - viewportY, x) = entry;
    }
}

static void vga_flush(void)
{
    if (renderPending)
    {
        vga_render();
        renderPending = false;
    }
}

static void vga_putchar_raw(char c)
{
    switch (c)
    {
    case '\t':
    {
        // Round cursorX up to the next multiple of 4
        cursorX = (cursorX + 4) & ~3;

        if (cursorX >= VGA_WIDTH)
        {
            cursorX = 0;
            ++cursorY;
        }

        break;
    }
    case '\n':
    {
        cursorX = 0;
        ++cursorY;
        break;
    }
    case '\r':
    {
        cursorX = 0;
        break;
    }
    case '\b':
    {
        if (cursorX > 0)
        {
            --cursorX;
            terminalBuffer[cursorY][cursorX] = VGA_CLEAR;
            vga_write_cell(cursorY, cursorX, VGA_CLEAR);
        }
        break;
    }
    default:
    {
        uint16_t entry = (uint16_t)(uint8_t)c | ((uint16_t)vgaColor << 8);

        terminalBuffer[cursorY][cursorX] = entry;
        vga_write_cell(cursorY, cursorX, entry);

        ++cursorX;

        // Move to the next line
        if (cursorX >= VGA_WIDTH)
        {
            cursorX = 0;
            ++cursorY;
        }

        break;
    }
    }

    // Move everything up by one line if we have reached the end
    if (cursorY >= TERMINAL_LINES)
    {
        // Copy the last line and put it to the penultimate line
        for (size_t y = 1; y < TERMINAL_LINES; ++y)
        {
            for (size_t x = 0; x < VGA_WIDTH; ++x)
            {
                terminalBuffer[y - 1][x] = terminalBuffer[y][x];
            }
        }

        // Clear the final line
        for (size_t x = 0; x < VGA_WIDTH; ++x)
        {
            terminalBuffer[TERMINAL_LINES - 1][x] = VGA_CLEAR;
        }

        cursorY = TERMINAL_LINES - 1;
        renderPending = true;
    }

    vga_follow_cursor();
}

static int vga_print_unsigned(unsigned int value, unsigned int base)
{
    char buffer[16];
    const char* digits = "0123456789abcdef";
    int cnt = 0;
    int idx = 0;

    if (value == 0u)
    {
        vga_putchar_raw('0');
        return 1;
    }

    while (value != 0u)
    {
        buffer[idx++] = digits[value % base];
        value /= base;
    }

    while (idx > 0)
    {
        vga_putchar_raw(buffer[--idx]);
        ++cnt;
    }

    return cnt;
}

static int vga_print_signed(int value)
{
    unsigned int mag;
    int cnt = 0;

    if (value < 0)
    {
        vga_putchar_raw('-');
        ++cnt;
        mag = (unsigned int)(-(value + 1)) + 1u; // two's complement
    }
    else
    {
        mag = (unsigned int)value;
    }

    return cnt + vga_print_unsigned(mag, 10u);
}

void vga_set_color(uint8_t fg, uint8_t bg)
{
    vgaColor = VGA_COLOR(fg, bg);
}

void vga_set_foreground(uint8_t fg)
{
    vgaColor = VGA_COLOR(fg, vgaColor >> 4);
}

void vga_set_background(uint8_t bg)
{
    vgaColor = VGA_COLOR(vgaColor & 0x0F, bg);
}

void vga_clear()
{
    for (size_t y = 0; y < TERMINAL_LINES; ++y)
    {
        for (size_t x = 0; x < VGA_WIDTH; ++x)
        {
            terminalBuffer[y][x] = VGA_CLEAR;
        }
    }

    cursorX = 0;
    cursorY = 0;
    viewportY = 0;

    for (uint8_t y = 0; y < VGA_HEIGHT; ++y)
    {
        for (uint8_t x = 0; x < VGA_WIDTH; ++x)
        {
            VGA_AT(y, x) = VGA_CLEAR;
        }
    }
}

void vga_putchar(char c)
{
    uint32_t flags = irq_save();
    vga_putchar_raw(c);
    vga_flush();
    irq_restore(flags);
}

void vga_write(const char* str)
{
    uint32_t flags = irq_save();
    while (*str)
    {
        vga_putchar_raw(*str);
        ++str;
    }
    vga_flush();
    irq_restore(flags);
}

void vga_scroll_up(void)
{
    if (viewportY > 0)
    {
        viewportY--;
        vga_render();
    }
}

void vga_scroll_down(void)
{
    size_t maxViewport = 0;

    if (cursorY >= VGA_HEIGHT)
    {
        maxViewport = cursorY - VGA_HEIGHT + 1;
    }

    if (viewportY < maxViewport)
    {
        ++viewportY;
        vga_render();
    }
}

void vga_page_up(void)
{
    if (viewportY >= VGA_HEIGHT)
    {
        viewportY -= VGA_HEIGHT;
    }
    else
    {
        viewportY = 0;
    }

    vga_render();
}

void vga_page_down(void)
{
    size_t maxViewport = 0;

    if (cursorY >= VGA_HEIGHT)
    {
        maxViewport = cursorY - VGA_HEIGHT + 1;
    }

    viewportY += VGA_HEIGHT;

    if (viewportY > maxViewport)
    {
        viewportY = maxViewport;
    }

    vga_render();
}

void vga_init(void)
{
    vga_clear();
}

int vvga_write(const char* fmt, va_list args)
{
    int cnt = 0;

    while (*fmt != '\0')
    {
        // Ordinary characters are written directly
        if (*fmt != '%')
        {
            vga_putchar_raw(*fmt++);
            ++cnt;
            continue;
        }

        // Skip the '%' character and check the format specifier
        ++fmt;

        switch (*fmt)
        {
        case '\0':
        {
            return cnt;
        }
        case '%':
        {
            vga_putchar_raw('%');
            ++cnt;
            break;
        }
        case 'c':
        {
            vga_putchar_raw(va_arg(args, int));
            ++cnt;
            break;
        }
        case 's':
        {
            const char* str = va_arg(args, const char*);
            if (str == 0)
            {
                str = "(null)";
            }

            while (*str != '\0')
            {
                vga_putchar_raw(*str++);
                ++cnt;
            }
            break;
        }
        case 'd':
        case 'i':
        {
            cnt += vga_print_signed(va_arg(args, int));
            break;
        }
        case 'u':
        {
            cnt += vga_print_unsigned(va_arg(args, unsigned int), 10u);
            break;
        }
        case 'x':
        {
            vga_putchar_raw('0');
            vga_putchar_raw('x');
            cnt += 2;
            cnt += vga_print_unsigned(va_arg(args, unsigned int), 16u);
            break;
        }
        default:
        {
            vga_putchar_raw('%');
            vga_putchar_raw(*fmt);
            cnt += 2;
            break;
        }
        }

        // Advance to the next character in the format string
        ++fmt;
    }

    return cnt;
}

int fvga_write(const char* fmt, ...)
{
    int cnt;
    va_list args;

    uint32_t flags = irq_save();
    va_start(args, fmt);
    cnt = vvga_write(fmt, args);
    va_end(args);
    vga_flush();
    irq_restore(flags);

    return cnt;
}
