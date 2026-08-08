#include "print.h"

#include <stdarg.h>
#include <stdint.h>

// @brief Prints a single character using BIOS teletype.
//
// Uses BIOS interrupt 0x10 with function 0x0E to display
// the input character on the active text-mode screen.
//
// @param c The character to print.
void bios_print_char(char c)
{
    // AH = 0x0E selects teletype output
    // AL = c is the character to print
    uint16_t ax = 0x0E00u | (unsigned char)c;

    // BH = 0x00 displays page 0
    // BL = 0x07 sets the text attribute to light gray
    uint16_t bx = 0x0007u;

    // Inline assembly
    __asm__ volatile (
        "int $0x10"
        :
        : "a"(ax), "b"(bx)  // loads ax into AX and bx into BX
    );
}

// @brief Prints a character using BIOS teletype.
//
// Uses BIOS interrupt 0x10 with function 0x0E to display
// the input string on the active text-mode screen.
//
// @param ch The character to print.
int putchar(int ch)
{
    // Put the cursor back to column 0 if the input character is a newline (EOL)
    if (ch == '\n')
    {
        bios_print_char('\r');
    }

    bios_print_char((unsigned char)ch);

    return (unsigned char)ch;
}

// @brief Prints a string using BIOS teletype.
//
// Uses BIOS interrupt 0x10 with function 0x0E to display
// the input string on the active text-mode screen.
//
// @param str The string to print.
void bios_print(const char* str)
{
    while (*str != '\0')
    {
        putchar((unsigned char)*str++);
    }
}

// @brief Renders an unsigned integer in the requested base (10 or 16).
// @param value Unsigned value to print.
// @param base Numeric base for conversion.
// @return Number of printed characters.
int print_unsigned(unsigned int value, unsigned int base)
{
    char buffer[16];
    const char* digits = "0123456789abcdef";
    int cnt = 0;
    int idx = 0;

    if (value == 0u)
    {
        return putchar('0');
    }

    while (value != 0u)
    {
        buffer[idx++] = digits[value % base];
        value /= base;
    }

    while (idx > 0)
    {
        cnt += (putchar(buffer[--idx]) >= 0) ? 1 : 0;
    }

    return cnt;
}

// @brief Renders a signed decimal integer.
// @param value Signed value to print.
// @return Number of printed characters.
int print_signed(int value)
{
    unsigned int mag;
    int cnt = 0;

    if (value < 0)
    {
        cnt += (putchar('-') >= 0) ? 1 : 0;
        mag = (unsigned int)(-(value + 1)) + 1u;    // two's complement
    }
    else
    {
        mag = (unsigned int)value;
    }

    return cnt + print_unsigned(mag, 10u);
}

// @brief Formats and prints a variadic argument list to the BIOS teletype output.
// @param fmt Null-terminated format string.
// @param args Variadic argument list matching the format string.
// @return Numbers of printed characters.
int vprintf(const char* fmt, va_list args)
{
    int cnt = 0;

    while (*fmt != '\0')
    {
        // Ordinary characters are written directly
        if (*fmt != '%')
        {
            cnt += (putchar(*fmt++) >= 0) ? 1 : 0;
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
                cnt += (putchar('%') >= 0) ? 1 : 0;
                break;
            }
            case 'c':
            {
                cnt += (putchar(va_arg(args, int)) >= 0) ? 1 : 0;
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
                    cnt += (putchar(*str++) >= 0) ? 1 : 0;
                }
                break;
            }
            case 'd':
            case 'i':
            {
                cnt += print_signed(va_arg(args, int));
                break;
            }
            case 'u':
            {
                cnt += print_unsigned(va_arg(args, unsigned int), 10u);
                break;
            }
            case 'x':
            {
                cnt += print_unsigned(va_arg(args, unsigned int), 16u);
                break;
            }
            default:
            {
                cnt += (putchar('%') >= 0) ? 1 : 0;
                cnt += (putchar(*fmt) >= 0) ? 1 : 0;
                break;
            }
        }

        // Advance to the next character in the format string
        ++fmt;
    }

    return cnt;
}

int printf(const char* fmt, ...)
{
    int cnt;
    va_list args;

    va_start(args, fmt);
    cnt = vprintf(fmt, args);
    va_end(args);

    return cnt;
}
