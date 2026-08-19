#ifndef KERNEL_SCHEDULER_SCHEDULER_H
#define KERNEL_SCHEDULER_SCHEDULER_H

#include <stdint.h>

typedef enum
{
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_TERMINATED
} task_state_t;

typedef struct task
{
    uint32_t id;
    uintptr_t esp; // stack address that represents the saved CPU context
    task_state_t state;

    struct task* next;
} task_t;

void scheduler_init(void);

/// @brief Creaets a new task.
///
/// @param entry Entry point for the new task.
/// @return Task ID on success; otherwise -1 on failure
int task_create(void (*entry)(void));

/// @brief Voluntarily gives up the CPU.
void task_yield(void);

/// @brief Terminates the currently-running task.
void task_exit(void);

/// @brief Blocks the current task that is running.
void task_block(void);

/// @brief Unblocks the current task that is being blocked.
///
/// @param id ID of the task to wake up.
/// @return 0 on success, -1 on failure.
int task_unblock(uint32_t id);

/// @brief Performs a round-robin context switch to the next task.
///
/// @param regs Pointer to the saved register of the current task.
/// @return Pointer to the saved register of the next task.
struct registers* scheduler_schedule(struct registers* regs);

/// @brief Handles a timer tick and determines whether the time slice of the current task has expired.
///
/// @param regs Pointer to the register state saved by the interrupt handler.
/// @return The register state that should be stored. It can either be that of the current task or
/// the next task.
struct registers* scheduler_tick(struct registers* regs);

/// @brief Marks the current task as terminated and invokes the scheduler
/// to select and switch to the next runnable task.
///
/// @param regs Pointer to the saved CPU register state.
/// @return The register state returned by the scheduler, or regs if the current task or register state is invalid.
struct registers* scheduler_exit(struct registers* regs);

#endif // KERNEL_SCHEDULER_SCHEDULER_H
