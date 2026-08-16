#include "vga.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/x86/io.h"

// VGA dimension
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define TERMINAL_LINES  1000

#define VGA_MEMORY ((volatile uint16_t *)0xB8000)

// Colours
#define VGA_BLACK         0x0
#define VGA_BLUE          0x1
#define VGA_GREEN         0x2
#define VGA_CYAN          0x3
#define VGA_RED           0x4
#define VGA_MAGENTA       0x5
#define VGA_BROWN         0x6
#define VGA_LIGHT_GRAY    0x7
#define VGA_DARK_GRAY     0x8
#define VGA_LIGHT_BLUE    0x9
#define VGA_LIGHT_GREEN   0xA
#define VGA_LIGHT_CYAN    0xB
#define VGA_LIGHT_RED     0xC
#define VGA_LIGHT_MAGENTA 0xD
#define VGA_YELLOW        0xE
#define VGA_WHITE         0xF

#define VGA_AT(row, col) VGA_MEMORY[(row) * VGA_WIDTH + (col)]

#define VGA_COLOR(fg, bg) ((uint8_t)(fg) | ((uint8_t)(bg) << 4))
#define VGA_ENTRY(ch, fg, bg) ((uint16_t)(ch) | (uint16_t)VGA_COLOR(fg, bg) << 8)
#define VGA_CLEAR VGA_ENTRY(' ', VGA_BLACK, VGA_BLACK)

static uint16_t terminalBuffer[TERMINAL_LINES][VGA_WIDTH];

static size_t cursorX = 0;
static size_t cursorY = 0;
static size_t viewportY = 0;

static void terminal_render(void)
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

static void terminal_follow_cursor(void)
{
    if (cursorY >= viewportY + VGA_HEIGHT)
    {
        viewportY = cursorY - VGA_HEIGHT + 1;
    }
}

static void terminal_putchar_raw(char c)
{
    switch (c)
    {
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
        }
        break;
    }
    default:
    {
        terminalBuffer[cursorY][cursorX] = VGA_ENTRY(c, VGA_WHITE, VGA_BLACK);

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
    }

    terminal_follow_cursor();
}

static int terminal_print_unsigned(unsigned int value, unsigned int base)
{
    char buffer[16];
    const char* digits = "0123456789abcdef";
    int cnt = 0;
    int idx = 0;

    if (value == 0u)
    {
        terminal_putchar_raw('0');
        return 1;
    }

    while (value != 0u)
    {
        buffer[idx++] = digits[value % base];
        value /= base;
    }

    while (idx > 0)
    {
        terminal_putchar_raw(buffer[--idx]);
        ++cnt;
    }

    return cnt;
}

static int terminal_print_signed(int value)
{
    unsigned int mag;
    int cnt = 0;

    if (value < 0)
    {
        terminal_putchar_raw('-');
        ++cnt;
        mag = (unsigned int)(-(value + 1)) + 1u;    // two's complement
    }
    else
    {
        mag = (unsigned int)value;
    }

    return cnt + terminal_print_unsigned(mag, 10u);
}

void terminal_clear()
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

void terminal_putchar(char c)
{
    terminal_putchar_raw(c);
    terminal_render();
}

void terminal_write(const char *str)
{
    uint32_t flags = irq_save();
    while (*str)
    {
        terminal_putchar_raw(*str);
        ++str;
    }
    irq_restore(flags);
}

void terminal_scroll_up(void)
{
    if (viewportY > 0) {
        viewportY--;
        terminal_render();
    }
}

void terminal_scroll_down(void)
{
    size_t maxViewport = 0;

    if (cursorY >= TERMINAL_LINES)
    {
        maxViewport = cursorY - VGA_HEIGHT + 1;
    }

    if (viewportY < maxViewport)
    {
        viewportY++;
        terminal_render();
    }
}

void terminal_page_up(void)
{
    // Check if the current viewpoint is greater than the VGA height
    viewportY -= (viewportY >= VGA_HEIGHT) ? VGA_HEIGHT : 0;

    terminal_render();
}

void terminal_page_down(void)
{
    size_t maxViewport = 0;

    if (cursorY >= VGA_HEIGHT)
    {
        maxViewport = cursorY - VGA_HEIGHT + 1;
    }

    viewportY += VGA_HEIGHT;

    // Clamp the viewport
    if (viewportY > maxViewport)
    {
        viewportY = maxViewport;
    }

    terminal_render();
}

void terminal_init(void)
{
    terminal_clear();
}

int vterminal_write(const char* fmt, va_list args)
{
    int cnt = 0;

    while (*fmt != '\0')
    {
        // Ordinary characters are written directly
        if (*fmt != '%')
        {
            terminal_putchar_raw(*fmt++);
            ++cnt;
            continue;
        }

        // Skip the '%' character and check the format specifier
        ++fmt;

        switch (*fmt)
        {
            case '\0':
            {
                return  cnt;
            }
            case '%':
            {
                terminal_putchar_raw('%');
                ++cnt;
                break;
            }
            case 'c':
            {
                terminal_putchar_raw(va_arg(args, int));
                ++cnt;
                break;
            }
            case 's':
            {
                const char *str = va_arg(args, const char*);
                if (str == 0)
                {
                    str = "(null)";
                }

                while (*str != '\0')
                {
                    terminal_putchar_raw(*str++);
                    ++cnt;
                }
                break;
            }
            case 'd':
            case 'i':
            {
                cnt += terminal_print_signed(va_arg(args, int));
                break;
            }
            case 'u':
            {
                cnt += terminal_print_unsigned(va_arg(args, unsigned int), 10u);
                break;
            }
            case 'x':
            {
                cnt += terminal_print_unsigned(va_arg(args, unsigned int), 16u);
                break;
            }
            default:
            {
                terminal_putchar_raw('%');
                terminal_putchar_raw(*fmt);
                cnt += 2;
                break;
            }
        }

        // Advance to the next character in the format string
        ++fmt;
    }

    return cnt;
}

int fterminal_write(const char* fmt, ...)
{
    int cnt;
    va_list args;

    uint32_t flags = irq_save();
    va_start(args, fmt);
    cnt = vterminal_write(fmt, args);
    va_end(args);
    irq_restore(flags);

    return cnt;
}
