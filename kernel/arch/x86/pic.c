#include "pic.h"

#include <stdint.h>

#include "io.h"

void pic_init(void)
{
    uint8_t masterMask = inb(PIC1_DATA);
    uint8_t slaveMask  = inb(PIC2_DATA);

    // Start initialization sequence
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // Set vector offsets
    // master: IRQ 0-7  -> 32-39
    // slave : IRQ 8-15 -> 40-47
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    // Tell Master PIC that Slave is connected to IRQ2
    outb(PIC1_DATA, 0x04);

    // Tell Slave PIC its cascade identity
    outb(PIC2_DATA, 0x02);

    // 8086 mode
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // Restore interrupt masks
    // outb(PIC1_DATA, masterMask);
    // outb(PIC2_DATA, slaveMask);

    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)
    {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    outb(PIC1_COMMAND, PIC_EOI);
}
