#ifndef KERNEL_ARCH_X86_GDT_H
#define KERNEL_ARCH_X86_GDT_H

#include <stdint.h>

#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10

#define GDT_USER_CODE 0x1B
#define GDT_USER_DATA 0x23

struct gdt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

//@brief Initializes and load the kernel GDT.
void gdt_init(void);

#endif // KERNEL_ARCH_X86_GDT_H
