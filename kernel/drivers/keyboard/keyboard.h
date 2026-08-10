#ifndef KERNEL_DRIVERS_KEYBOARD_KEYBOARD_H
#define KERNEL_DRIVERS_KEYBOARD_KEYBOARD_H

/// @brief Initializes the keyboard and enables its PIC interrupt (IRQ1).
void keyboard_init(void);

/// @brief Handles keyboard interrupts by reading and processing the keyboard scancode.
void keyboard_handler(void);

#endif  // KERNEL_DRIVERS_KEYBOARD_KEYBOARD_H
