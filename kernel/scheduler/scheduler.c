#include "scheduler.h"
#include "arch/x86/idt.h"
#include "mm/kmalloc.h"

#include <stdint.h>
#include <stddef.h>

#define MAX_TASKS       10
#define MAX_USER_TASKS  ((MAX_TASKS) - 1)
#define STACK_SIZE      4096
#define TIME_SLICE_MS   10

static task_t tasks[MAX_TASKS];

__attribute__((aligned(16))) static uint8_t taskStacks[MAX_USER_TASKS][STACK_SIZE];

static task_t* curTask = NULL;

static uint32_t taskCnt = 0;

static uint32_t schedulerTicks = 0;

static uintptr_t task_create_context(uint8_t *stack, void (*entry)(void))
{
    uintptr_t *sp = (uintptr_t *)(stack + STACK_SIZE);

    *(--sp) = (uintptr_t)task_exit;

    *(--sp) = 0x202;              // EFLAGS: IF = 1
    *(--sp) = 0x08;               // Kernel CS
    *(--sp) = (uintptr_t)entry;   // EIP

    *(--sp) = 0;                  // error code
    *(--sp) = 32;                 // interrupt number

    *(--sp) = 0;                  // EAX
    *(--sp) = 0;                  // ECX
    *(--sp) = 0;                  // EDX
    *(--sp) = 0;                  // EBX
    *(--sp) = 0;                  // original ESP
    *(--sp) = 0;                  // EBP
    *(--sp) = 0;                  // ESI
    *(--sp) = 0;                  // EDI

    return (uintptr_t)sp;
}

void scheduler_init(void)
{
    for (uint8_t i = 0; i < MAX_TASKS; ++i)
    {
        tasks[i].id = 0;
        tasks[i].state = TASK_TERMINATED;
        tasks[i].esp = 0;
        tasks[i].next = NULL;
    }

    taskCnt = 1;

    // The kernel task
    tasks[0].state = TASK_RUNNING;

    curTask = &tasks[0];
    schedulerTicks = 0;
}

int task_create(void (*entry)(void))
{
    // Make sure the task is valid
    if (!entry)
    {
        return -1;
    }

    task_t* slot = NULL;
    for (uint32_t i = 1; i < MAX_TASKS; ++i)
    {
        if (tasks[i].state == TASK_TERMINATED || tasks[i].id == 0)
        {
            slot = &tasks[i];
            break;
        }
    }

    if (!slot)
    {
        return -1;
    }

    // Only assign a fresh ID for a brand new task
    if (slot->id == 0)
    {
        slot->id = taskCnt++;
    }

    slot->state = TASK_READY;
    slot->esp = task_create_context(taskStacks[slot->id - 1], entry);

    return (int)slot->id;
}

void task_yield(void)
{
    asm volatile ("int $49");
}

void task_exit(void)
{
    curTask->state = TASK_TERMINATED;

    asm volatile("int $50");

    for (;;)
    {
        asm volatile ("hlt");
    }
}

struct registers *scheduler_schedule(struct registers *regs)
{
    if (!curTask || !regs || taskCnt == 0)
    {
        return regs;
    }

    curTask->esp = (uintptr_t)regs;

    // If the current task is running, then demote it to ready
    // since it has already run  and needs to be back in queue
    if (curTask->state == TASK_RUNNING)
    {
        curTask->state = TASK_READY;
    }

    uint32_t curIdx = 0;

    // Find the index of the current task
    for (uint32_t i = 0; i < MAX_TASKS; ++i)
    {
        if (curTask == &tasks[i])
        {
            curIdx = i;
            break;
        }
    }

    // Round-robin search
    // i starts at 1 since we want to avoid checking the current task itself again
    for (uint32_t i = 1; i < MAX_TASKS; ++i)
    {
        uint32_t nextIdx = (curIdx + i) % MAX_TASKS;

        if (tasks[nextIdx].state == TASK_READY)
        {
            // Perform the context switch
            curTask = &tasks[nextIdx];
            curTask->state = TASK_RUNNING;

            // Reset the time slice for the new task
            schedulerTicks = 0;

            return (struct registers *)curTask->esp;
        }
    }

    // If the current task is ready, it is safe to continue to run it
    if (curTask->state == TASK_READY)
    {
        curTask->state = TASK_RUNNING;
        schedulerTicks = 0;

        return (struct registers*)curTask->esp;
    }

    // There is no task that is ready, then we halt
    // until an interrupt fires and then loop back to check the new task
    for (;;)
    {
        asm volatile ("sti; hlt");

        for (uint32_t i = 0; i < MAX_TASKS; ++i)
        {
            if (tasks[i].state == TASK_READY)
            {
                // Perform the context switch
                curTask = &tasks[i];
                curTask->state = TASK_RUNNING;

                // Reset the time slice for the new task
                schedulerTicks = 0;

                return (struct registers *)curTask->esp;
            }
        }
    }
}

struct registers* scheduler_tick(struct registers *regs)
{
    ++schedulerTicks;

    // Check if the current task is still within its allocated time slice
    if (schedulerTicks < TIME_SLICE_MS)
    {
        return regs;
    }

    // The current task has excedded its allocation, need to schedule the next task
    schedulerTicks = 0;

    return scheduler_schedule(regs);
}

struct registers *scheduler_exit(struct registers *regs)
{
    if (!curTask || !regs)
    {
        return regs;
    }

    // Mark the task terminated before the scheduler
    curTask->state = TASK_TERMINATED;

    return scheduler_schedule(regs);
}
