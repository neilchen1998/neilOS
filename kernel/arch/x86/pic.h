#ifndef KERNEL_ARCH_x86_PIC_H
#define KERNEL_ARCH_x86_PIC_H

#include <stdint.h>

#define PIC1        0x20
#define PIC2        0xA0

#define PIC1_COMMAND PIC1
#define PIC1_DATA    (PIC1 + 1)

#define PIC2_COMMAND PIC2
#define PIC2_DATA    (PIC2 + 1)

#define PIC_EOI      0x20

/// @brief Initializes and remaps the Programmable Interrupt Controller (PIC).
void pic_init(void);

/// @brief Signals the PIC that the specified hardware interrupt has been handled.
/// @param irq The IRQ number you want to send the EOI for.
void pic_send_eoi(uint8_t irq);

#endif  // KERNEL_ARCH_x86_PIC_H
