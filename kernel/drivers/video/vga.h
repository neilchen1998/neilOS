#ifndef KERNEL_DRIVERS_VIDEO_VGA_H
#define KERNEL_DRIVERS_VIDEO_VGA_H

#include <stdint.h>

/// @brief Clears the terminal screen and terminal buffer.
void terminal_clear(void);

/// @brief Writes a null-terminated string to the terminal.
/// @param str The string to write.
void terminal_write(const char *str);

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

#endif  // KERNEL_DRIVERS_VIDEO_VGA_H
