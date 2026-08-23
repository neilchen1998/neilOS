#ifndef KERNEL_DRIVERS_VIDEO_VGA_H
#define KERNEL_DRIVERS_VIDEO_VGA_H

#include <stdarg.h>
#include <stdint.h>

// Colours
#define VGA_BLACK 0x0
#define VGA_BLUE 0x1
#define VGA_GREEN 0x2
#define VGA_CYAN 0x3
#define VGA_RED 0x4
#define VGA_MAGENTA 0x5
#define VGA_BROWN 0x6
#define VGA_LIGHT_GRAY 0x7
#define VGA_DARK_GRAY 0x8
#define VGA_LIGHT_BLUE 0x9
#define VGA_LIGHT_GREEN 0xA
#define VGA_LIGHT_CYAN 0xB
#define VGA_LIGHT_RED 0xC
#define VGA_LIGHT_MAGENTA 0xD
#define VGA_YELLOW 0xE
#define VGA_WHITE 0xF

/// @brief Sets both the foreground and background terminal colours.
///
/// @param fg Foreground colour.
/// @param bg Background colour.
void terminal_set_color(uint8_t fg, uint8_t bg);

/// @brief Sets the foreground terminal colours.
///
/// @param fg Foreground colour.
void terminal_set_foreground(uint8_t fg);

/// @brief Sets the background terminal colours.
///
/// @param bg Background colour.
void terminal_set_background(uint8_t bg);

/// @brief Clears the terminal screen and terminal buffer.
void terminal_clear(void);

/// @brief Writes a null-terminated string to the terminal.
/// @param str The string to write.
void terminal_putchar(char c);

/// @brief Writes a null-terminated string to the terminal.
/// @param str The string to write.
void terminal_write(const char* str);

/// @brief Scrolls up the terminal viewport by one line.
void terminal_scroll_up(void);

/// @brief Scrolls down the terminal viewport by one line.
void terminal_scroll_down(void);

/// @brief Scrolls up the terminal viewport by one page.
void terminal_page_up(void);

/// @brief Scrolls down the terminal viewport by one page.
void terminal_page_down(void);

/// @brief Initializes the terminal.
void terminal_init(void);

// @brief Formats and prints a variadic argument list to the terminal.
// @param fmt Null-terminated format string.
// @param args Variadic argument list matching the format string.
// @return Numbers of printed characters on success or a negative value on failure.
int vterminal_write(const char* fmt, va_list args);

// @briefPrints formatted string to the terminal.
// @param fmt Null-terminated format string followed by values referenced by fmt.
// @return Numbers of printed characters on success or a negative value on failure.
int fterminal_write(const char* fmt, ...);

#endif // KERNEL_DRIVERS_VIDEO_VGA_H
