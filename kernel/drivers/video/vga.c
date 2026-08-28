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
static uint8_t terminalColor = VGA_COLOR(VGA_WHITE, VGA_BLACK);

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
        renderPending = true;
    }
}

static void terminal_write_cell(size_t bufferY, size_t x, uint16_t entry)
{
    if (!renderPending && bufferY >= viewportY && bufferY < viewportY + VGA_HEIGHT)
    {
        VGA_AT(bufferY - viewportY, x) = entry;
    }
}

static void terminal_flush(void)
{
    if (renderPending)
    {
        terminal_render();
        renderPending = false;
    }
}

static void terminal_putchar_raw(char c)
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
            terminal_write_cell(cursorY, cursorX, VGA_CLEAR);
        }
        break;
    }
    default:
    {
        uint16_t entry = (uint16_t)(uint8_t)c | ((uint16_t)terminalColor << 8);

        terminalBuffer[cursorY][cursorX] = entry;
        terminal_write_cell(cursorY, cursorX, entry);

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
        mag = (unsigned int)(-(value + 1)) + 1u; // two's complement
    }
    else
    {
        mag = (unsigned int)value;
    }

    return cnt + terminal_print_unsigned(mag, 10u);
}

void terminal_set_color(uint8_t fg, uint8_t bg)
{
    terminalColor = VGA_COLOR(fg, bg);
}

void terminal_set_foreground(uint8_t fg)
{
    terminalColor = VGA_COLOR(fg, terminalColor >> 4);
}

void terminal_set_background(uint8_t bg)
{
    terminalColor = VGA_COLOR(terminalColor & 0x0F, bg);
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
    uint32_t flags = irq_save();
    terminal_putchar_raw(c);
    terminal_flush();
    irq_restore(flags);
}

void terminal_write(const char* str)
{
    uint32_t flags = irq_save();
    while (*str)
    {
        terminal_putchar_raw(*str);
        ++str;
    }
    terminal_flush();
    irq_restore(flags);
}

void terminal_scroll_up(void)
{
    if (viewportY > 0)
    {
        viewportY--;
        terminal_render();
    }
}

void terminal_scroll_down(void)
{
    size_t maxViewport = 0;

    if (cursorY >= VGA_HEIGHT)
    {
        maxViewport = cursorY - VGA_HEIGHT + 1;
    }

    if (viewportY < maxViewport)
    {
        ++viewportY;
        terminal_render();
    }
}

void terminal_page_up(void)
{
    if (viewportY >= VGA_HEIGHT)
    {
        viewportY -= VGA_HEIGHT;
    }
    else
    {
        viewportY = 0;
    }

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
            return cnt;
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
            const char* str = va_arg(args, const char*);
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
            terminal_putchar_raw('0');
            terminal_putchar_raw('x');
            cnt += 2;
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
    terminal_flush();
    irq_restore(flags);

    return cnt;
}
