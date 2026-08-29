#include "syscall.h"

#include <stdint.h>

#include "arch/x86/idt.h"
#include "drivers/timer/pit.h"
#include "scheduler/scheduler.h"

struct registers* syscall_dispatch(struct registers* regs)
{
    if (!regs)
    {
        return regs;
    }

    switch (regs->eax)
    {
    case SYS_EXIT:
    {
        return scheduler_exit(regs);
    }
    case SYS_YIELD:
    {
        return scheduler_schedule(regs);
    }
    // case SYS_GETPID:
    // {
    //     return regs;
    // }
    case SYS_GET_TICKS:
    {
        regs->eax = pit_get_ticks();
        return regs;
    }
    // case SYS_SLEEP:
    // {
    //     return regs;
    // }
    // case SYS_WRITE:
    // {
    //     return regs;
    // }
    // case SYS_READ:
    // {
    //     return regs;
    // }
    // case SYS_SBRK:
    // {
    //     return regs;
    // }
    // case SYS_MMAP:
    // {
    //     return regs;
    // }
    // case SYS_MUNMAP:
    // {
    //     return regs;
    // }
    // case SYS_OPEN:
    // {
    //     return regs;
    // }
    // case SYS_CLOSE:
    // {
    //     return regs;
    // }
    // case SYS_READ_FILE:
    // {
    //     return regs;
    // }
    // case SYS_WRITE_FILE:
    // {
    //     return regs;
    // }
    // case SYS_SEEK:
    // {
    //     return regs;
    // }
    // case SYS_STAT:
    // {
    //     return regs;
    // }
    // case SYS_FORK:
    // {
    //     return regs;
    // }
    // case SYS_EXEC:
    // {
    //     return regs;
    // }
    // case SYS_WAIT:
    // {
    //     return regs;
    // }
    // case SYS_GET_TIME:
    // {
    //     return regs;
    // }
    // case SYS_GET_MEMINFO:
    // {
    //     return regs;
    // }
    // case SYS_COMPLETE:
    // {
    //     return regs;
    // }
    default:
    {
        regs->eax = (uint32_t)-1;
        return regs;
    }
    }
}
