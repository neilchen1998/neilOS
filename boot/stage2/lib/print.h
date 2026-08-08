#ifndef BOOT_STAGE2_PRINT_H
#define BOOT_STAGE2_PRINT_H

#include <stdarg.h>
#include <stdint.h>

// @brief Prints a single character using BIOS teletype.
//
// Uses BIOS interrupt 0x10 with function 0x0E to display
// the input character on the active text-mode screen.
//
// @param c The character to print.
void bios_print_char(char c);

// @brief Prints a character using BIOS teletype.
//
// Uses BIOS interrupt 0x10 with function 0x0E to display
// the input string on the active text-mode screen.
//
// @param ch The character to print.
int putchar(int ch);

// @brief Prints a string using BIOS teletype.
//
// Uses BIOS interrupt 0x10 with function 0x0E to display
// the input string on the active text-mode screen.
//
// @param str The string to print.
void bios_print(const char* str);

// @brief Renders an unsigned integer in the requested base (10 or 16).
// @param value Unsigned value to print.
// @param base Numeric base for conversion.
// @return Number of printed characters.
int print_unsigned(unsigned int value, unsigned int base);

// @brief Renders a signed decimal integer.
// @param value Signed value to print.
// @return Number of printed characters.
int print_signed(int value);

// @brief Formats and prints a variadic argument list to the BIOS teletype output.
// @param fmt Null-terminated format string.
// @param args Variadic argument list matching the format string.
// @return Numbers of printed characters.
int vprintf(const char* fmt, va_list args);

int printf(const char* fmt, ...);

#endif
