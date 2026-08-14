#include "scheduler.h"

#include <stdint.h>
#include <stddef.h>

#include "drivers/video/vga.h"

#define MAX_TASKS       3
#define MAX_USER_TASKS  ((MAX_TASKS) - 1)
#define STACK_SIZE      4096
#define TIME_SLICE_MS   10

static task_t tasks[MAX_TASKS];

static uint8_t taskStacks[MAX_USER_TASKS][STACK_SIZE];

static task_t* curTask = NULL;

static uint32_t taskCnt = 0;

static uint32_t schedulerTicks = 0;

static void task_a(void)
{
    for (;;)
    {
        terminal_write("A");
    }
}

static void task_b(void)
{
    for (;;)
    {
        terminal_write("B");
    }
}

static uint32_t task_create_context(uint8_t *stack, void (*entry)(void))
{
    uintptr_t *sp = (uintptr_t *)(stack + STACK_SIZE);

    *(--sp) = 0x202;        // EFLAGS: IF = 1
    *(--sp) = 0x08;         // kernel code segment
    *(--sp) = (uintptr_t)entry;

    *(--sp) = 0;             // error code
    *(--sp) = 32;            // fake interrupt number

    *(--sp) = 0;             // eax
    *(--sp) = 0;             // ecx
    *(--sp) = 0;             // edx
    *(--sp) = 0;             // ebx
    *(--sp) = 0;             // original ESP
    *(--sp) = 0;             // ebp
    *(--sp) = 0;             // esi
    *(--sp) = 0;             // edi

    return (uintptr_t)sp;
}

void scheduler_init(void)
{
    taskCnt = 3;

    // The kernel task
    tasks[0].id = 0;
    tasks[0].state = TASK_RUNNING;
    tasks[0].esp = 0;

    // Dummy task A
    tasks[1].id = 1;
    tasks[1].state = TASK_READY;
    tasks[1].esp = task_create_context(taskStacks[0], task_a);

    // Dummy task B
    tasks[2].id = 2;
    tasks[2].state = TASK_READY;
    tasks[2].esp = task_create_context(taskStacks[1], task_b);

    curTask = &tasks[0];
}

struct registers *scheduler_schedule(struct registers *regs)
{
    curTask->esp = (uintptr_t)regs;
    curTask->state = TASK_READY;

    uint32_t next_id = curTask->id + 1;

    if (next_id >= taskCnt)
    {
        next_id = 0;
    }

    curTask = &tasks[next_id];
    curTask->state = TASK_RUNNING;

    return (struct registers*)curTask->esp;
}

struct registers* scheduler_tick(struct registers *regs)
{
    ++schedulerTicks;

    if (schedulerTicks < TIME_SLICE_MS)
    {
        return regs;
    }

    schedulerTicks = 0;

    return scheduler_schedule(regs);
}
