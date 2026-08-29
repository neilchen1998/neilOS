#include "idt.h"

#include <stdint.h>

#include "arch/x86/pic.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/timer/pit.h"
#include "drivers/video/vga.h"
#include "scheduler/scheduler.h"
#include "syscall/syscall.h"

#define IDT_ENTRIES 256

static struct idt_entry idt[IDT_ENTRIES]; // x86 IDT contains 256 entries
static struct idt_ptr idtp;

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

extern void isr32(void); // IRQ 0
extern void isr33(void); // IRQ 1 (keyboard)
extern void isr34(void);
extern void isr35(void);
extern void isr36(void);
extern void isr37(void);
extern void isr38(void);
extern void isr39(void);
extern void isr40(void);
extern void isr41(void);
extern void isr42(void);
extern void isr43(void);
extern void isr44(void);
extern void isr45(void);
extern void isr46(void);
extern void isr47(void);

extern void isr49(void); // software yield interrupt
extern void isr50(void); // software exit interrupt

/// @brief Sets an entry in the interrrupt descriptor table (IDT).
//
// @param n Index of the IDT entry to configure.
// @param ptr Pointer to the interrupt handler function.
static void idt_set_gate(uint8_t n, void (*ptr)(void))
{
    uint32_t handler = (uint32_t)(uintptr_t)ptr;

    idt[n].offset_low = handler & 0xFFFF;
    idt[n].selector = 0x08;
    idt[n].zero = 0;
    idt[n].type_attr = 0x8E;
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

/// @brief Initializes the interrrupt descriptor table (IDT).
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
    idt_set_gate(1, isr1);
    idt_set_gate(2, isr2);
    idt_set_gate(3, isr3);
    idt_set_gate(4, isr4);
    idt_set_gate(5, isr5);
    idt_set_gate(6, isr6);
    idt_set_gate(7, isr7);
    idt_set_gate(8, isr8);
    idt_set_gate(9, isr9);
    idt_set_gate(10, isr10);
    idt_set_gate(11, isr11);
    idt_set_gate(12, isr12);
    idt_set_gate(13, isr13);
    idt_set_gate(14, isr14);
    idt_set_gate(15, isr15);
    idt_set_gate(16, isr16);
    idt_set_gate(17, isr17);
    idt_set_gate(18, isr18);
    idt_set_gate(19, isr19);
    idt_set_gate(20, isr20);
    idt_set_gate(21, isr21);
    idt_set_gate(22, isr22);
    idt_set_gate(23, isr23);
    idt_set_gate(24, isr24);
    idt_set_gate(25, isr25);
    idt_set_gate(26, isr26);
    idt_set_gate(27, isr27);
    idt_set_gate(28, isr28);
    idt_set_gate(29, isr29);
    idt_set_gate(30, isr30);
    idt_set_gate(31, isr31);

    idt_set_gate(32, isr32); // IRQ 0
    idt_set_gate(33, isr33); // IRQ 1 keyboard
    idt_set_gate(34, isr34);
    idt_set_gate(35, isr35);
    idt_set_gate(36, isr36);
    idt_set_gate(37, isr37);
    idt_set_gate(38, isr38);
    idt_set_gate(39, isr39);
    idt_set_gate(40, isr40);
    idt_set_gate(41, isr41);
    idt_set_gate(42, isr42);
    idt_set_gate(43, isr43);
    idt_set_gate(44, isr44);
    idt_set_gate(45, isr45);
    idt_set_gate(46, isr46);
    idt_set_gate(47, isr47);

    idt_set_gate(49, isr49); // software yield interrupt
    idt_set_gate(50, isr50); // software exit interrupt

    idtp.limit = sizeof(idt) - 1;
    idtp.base = (uint32_t)(uintptr_t)idt; // stores the memory address of the IDT to base

    // Load the Interrupt Descriptor Table (IDT)
    __asm__ volatile("lidt %0" : : "m"(idtp));
}

/// @brief Handles a CPU interrupt
//
// @param regs Pointer to the register state captured when the interrupt occurred.
struct registers* interrupt_handler(struct registers* regs)
{
    switch (regs->int_no)
    {
    case 0:
        vga_write("EXCEPTION: DIVIDE BY ZERO\n");
        break;

    case 1:
        vga_write("EXCEPTION: DEBUG\n");
        break;

    case 2:
        vga_write("EXCEPTION: NMI\n");
        break;

    case 3:
        vga_write("EXCEPTION: BREAKPOINT\n");
        break;

    case 4:
        vga_write("EXCEPTION: OVERFLOW\n");
        break;

    case 5:
        vga_write("EXCEPTION: BOUND RANGE EXCEEDED\n");
        break;

    case 6:
        vga_write("EXCEPTION: INVALID OPCODE\n");
        break;

    case 7:
        vga_write("EXCEPTION: DEVICE NOT AVAILABLE\n");
        break;

    case 8:
        vga_write("EXCEPTION: DOUBLE FAULT\n");
        break;

    case 9:
        vga_write("EXCEPTION: COPROCESSOR SEGMENT OVERRUN\n");
        break;

    case 10:
        vga_write("EXCEPTION: INVALID TSS\n");
        break;

    case 11:
        vga_write("EXCEPTION: SEGMENT NOT PRESENT\n");
        break;

    case 12:
        vga_write("EXCEPTION: STACK-SEGMENT FAULT\n");
        break;

    case 13:
        vga_write("EXCEPTION: GENERAL PROTECTION FAULT\n");
        break;

    case 14:
        vga_write("EXCEPTION: PAGE FAULT\n");
        break;

    case 16:
        vga_write("EXCEPTION: x87 FLOATING-POINT\n");
        break;

    case 17:
        vga_write("EXCEPTION: ALIGNMENT CHECK\n");
        break;

    case 18:
        vga_write("EXCEPTION: MACHINE CHECK\n");
        break;

    case 19:
        vga_write("EXCEPTION: SIMD FLOATING-POINT\n");
        break;

    case 20:
        vga_write("EXCEPTION: VIRTUALIZATION\n");
        break;

    case 21:
        vga_write("EXCEPTION: CONTROL PROTECTION\n");
        break;

    case 28:
        vga_write("EXCEPTION: HYPERVISOR INJECTION\n");
        break;

    case 29:
        vga_write("EXCEPTION: VMM COMMUNICATION\n");
        break;

    case 30:
        vga_write("EXCEPTION: SECURITY\n");
        break;

    case 32:
        pit_tick();
        pic_send_eoi(0); // EOI for IRQ 0 (PIT timer)

        return scheduler_tick(regs);

    case 33:
        keyboard_handler();
        pic_send_eoi(1); // EOI for IRQ 1 (keyboard)
        return regs;

    case 49:
        // software yield interrupt
        return scheduler_schedule(regs);

    case 50:
        // software exit interrupt
        return scheduler_exit(regs);

    case SYSCALL_VECTOR:
    {
        return syscall_dispatch(regs);
    }

    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
        pic_send_eoi(regs->int_no - 32);
        break;

    default:
        vga_write("UNKNOWN INTERRUPT\n");
        break;
    }

    return regs;
}
