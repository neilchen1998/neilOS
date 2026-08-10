#include "idt.h"

#include <stdint.h>

#ifdef DEBUG
#include "drivers/video/vga.h"
#endif

#define IDT_ENTRIES 256

static struct idt_entry idt[IDT_ENTRIES];   // x86 IDT contains 256 entries
static struct idt_ptr idtp;

extern void isr0(void);

static void idt_set_gate(uint8_t n, void (*ptr)(void))
{
    uint32_t handler = (uint32_t)(uintptr_t)ptr;

    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = 0x08;
    idt[n].zero        = 0;
    idt[n].type_attr   = 0x8E;
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

void idt_init(void)
{
    for (int i = 0; i < IDT_ENTRIES; ++i)
    {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].type_attr = 0;
        idt[i].offset_high = 0;
    }

    idt_set_gate(0, isr0);

    idtp.limit = sizeof(idt) - 1;
    idtp.base = (uint32_t)(uintptr_t)idt;   // stores the memory address of the IDT to base

    // Load the Interrupt Descriptor Table (IDT)
    __asm__ volatile ("lidt %0" : : "m"(idtp));
}

#ifdef DEBUG
void isr0_handler(void)
{
    terminal_write("ISR 0 FIRED!\n");
}
#endif
