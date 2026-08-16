#ifndef KERNEL_ARCH_X86_IO_H
#define KERNEL_ARCH_X86_IO_H

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

static inline unsigned char inb(uint16_t port)
{
    unsigned char value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t irq_save(void)
{
    uint32_t flags;
    __asm__ volatile("pushf; pop %0; cli": "=r"(flags));
    return flags;
}

static inline uint32_t irq_restore(uint32_t flags)
{
    __asm__ volatile("push %0; popf" :: "r"(flags));
}

#endif  // KERNEL_ARCH_X86_IO_H
