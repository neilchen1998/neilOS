#ifndef KERNEL_SCHEDULER_SCHEDULER_H
#define KERNEL_SCHEDULER_SCHEDULER_H

#include <stdint.h>

typedef enum
{
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED
} task_state_t;

typedef struct task
{
    uint32_t id;
    uintptr_t esp;  // stack address that represents the saved CPU context
    task_state_t state;

    struct task *next;
} task_t;

void scheduler_init(void);

/// @brief Performs a round-robin context switch to the next task.
///
/// @param regs Pointer to the saved register of the current task.
/// @return Pointer to the saved register of the next task.
struct registers *scheduler_schedule(struct registers *regs);

/// @brief Handles a timer tick and determines whether the time slice of the current task has expired.
///
/// @param regs Pointer to the register state saved by the interrupt handler.
/// @return The register state that should be stored. It can either be that of the current task or
/// the next task.
struct registers *scheduler_tick(struct registers *regs);

#endif  // KERNEL_SCHEDULER_SCHEDULER_H
