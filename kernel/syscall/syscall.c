#include "syscall.h"

#include <stdint.h>

#include "arch/x86/idt.h"
#include "scheduler/scheduler.h"

struct registers* syscall_dispatch(struct registers* regs)
{
    if (!regs)
    {
        return regs;
    }

    switch (regs->eax)
    {
    case SYS_YIELD:
    {
        return scheduler_schedule(regs);
    }

    default:
    {
        regs->eax = (uint32_t)-1;
        return regs;
    }
    }
}
