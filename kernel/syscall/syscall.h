#ifndef KERNEL_SYSCALL_SYSCALL_H
#define KERNEL_SYSCALL_SYSCALL_H

#include <stdint.h>

#define SYSCALL_VECTOR 0x80u

struct registers;

typedef enum
{
    // Process
    SYS_EXIT = 1,
    SYS_YIELD,
    SYS_GETPID,

    // Time
    SYS_GET_TICKS,
    SYS_SLEEP,

    // I/O
    SYS_WRITE,
    SYS_READ,

    // Memory
    SYS_SBRK,
    SYS_MMAP,
    SYS_MUNMAP,

    // File system
    SYS_OPEN,
    SYS_CLOSE,
    SYS_READ_FILE,
    SYS_WRITE_FILE,
    SYS_SEEK,
    SYS_STAT,

    // Process management
    SYS_FORK,
    SYS_EXEC,
    SYS_WAIT,

    // Kernel system info
    SYS_GET_TIME,
    SYS_GET_MEMINFO,

    // Kernel scheduler
    SYS_COMPLETE

} syscall_number_t;

// @brief Handles a system call requested throught interrupt 0x80.
//
// @param regs Pointer to the saved CPU register state.
// @return Pointer to the register state, with the syscall return value
// stored in the appropriate register.
struct registers* syscall_dispatch(struct registers* regs);

#endif // KERNEL_SYSCALL_SYSCALL_H
