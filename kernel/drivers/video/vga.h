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

/// @brief Sets both the foreground and background vga colours.
///
/// @param fg Foreground colour.
/// @param bg Background colour.
void vga_set_color(uint8_t fg, uint8_t bg);

/// @brief Sets the foreground vga colours.
///
/// @param fg Foreground colour.
void vga_set_foreground(uint8_t fg);

/// @brief Sets the background vga colours.
///
/// @param bg Background colour.
void vga_set_background(uint8_t bg);

/// @brief Clears the vga screen and vga buffer.
void vga_clear(void);

/// @brief Writes a null-terminated string to the vga.
/// @param str The string to write.
void vga_putchar(char c);

/// @brief Writes a null-terminated string to the vga.
/// @param str The string to write.
void vga_write(const char* str);

/// @brief Scrolls up the vga viewport by one line.
void vga_scroll_up(void);

/// @brief Scrolls down the vga viewport by one line.
void vga_scroll_down(void);

/// @brief Scrolls up the vga viewport by one page.
void vga_page_up(void);

/// @brief Scrolls down the vga viewport by one page.
void vga_page_down(void);

/// @brief Initializes the vga.
void vga_init(void);

// @brief Formats and prints a variadic argument list to the vga.
// @param fmt Null-terminated format string.
// @param args Variadic argument list matching the format string.
// @return Numbers of printed characters on success or a negative value on failure.
int vvga_write(const char* fmt, va_list args);

// @briefPrints formatted string to the vga.
// @param fmt Null-terminated format string followed by values referenced by fmt.
// @return Numbers of printed characters on success or a negative value on failure.
int fvga_write(const char* fmt, ...);

#endif // KERNEL_DRIVERS_VIDEO_VGA_H
