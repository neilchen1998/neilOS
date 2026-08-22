#ifndef KERNEL_ARCH_X86_IO_H
#define KERNEL_ARCH_X86_IO_H

#include <stdint.h>

// @brief Writes an 8-bit value to the specified I/O port.
//
// @param port I/O port address to write to.
// @param value 8-bit value to write to the port.
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

// @bried Writes an 8-bit value to the specified I/O port.
//
// @para port I/O port address to read from.
// @return The 8-bit value read from the port.
static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port) : "memory");
    return value;
}

// @bried Saves the current CPU interrupt flags and disable interrupts.
//
// @return The CPU interrupt flags captured before interrupts were disabled.
static inline uint32_t irq_save(void)
{
    uint32_t flags;
    __asm__ volatile("pushf; pop %0; cli" : "=r"(flags) : : "memory");
    return flags;
}

// @bried Restores the CPU interrupt flags, including the previous interrupt state.
//
// @para flags Interrupt flags previously returned by irq_save().
static inline void irq_restore(uint32_t flags)
{
    __asm__ volatile("push %0; popf" ::"r"(flags) : "memory");
}

#endif // KERNEL_ARCH_X86_IO_H
