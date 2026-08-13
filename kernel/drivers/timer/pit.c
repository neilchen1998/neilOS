#include "pit.h"

#include <stdint.h>

#include "arch/x86/io.h"

#define PIT_CHANNEL0   0x40
#define PIT_COMMAND    0x43
#define PIT_FREQUENCY  1193182  // Intel 8253/8254 PIT

static volatile uint32_t ticks = 0;

void pit_init(uint32_t frequency)
{
    uint32_t divisor = PIT_FREQUENCY / frequency;

    outb(PIT_COMMAND, 0x36);

    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

void pit_tick()
{
    ++ticks;
}

uint32_t pit_get_ticks()
{
    return ticks;
}
