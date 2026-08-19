#include "scheduler.h"

#include <stddef.h>
#include <stdint.h>

#include "arch/x86/idt.h"
#include "arch/x86/io.h"

#define MAX_TASKS 10
#define MAX_USER_TASKS ((MAX_TASKS) - 1)
#define STACK_SIZE 4096
#define TIME_SLICE_MS 10

static task_t tasks[MAX_TASKS];

__attribute__((aligned(16))) static uint8_t taskStacks[MAX_USER_TASKS][STACK_SIZE];

// A dedicated idle task that runs ONLY when nothing else is ready
// NOTE: this is needed when the scheduler never has to halt inside the interrrupt handler
static task_t idleTask;

__attribute__((aligned(16))) static uint8_t idleStack[STACK_SIZE];

static volatile task_t* curTask = NULL;

static uint32_t nextTaskID = 0;

static volatile uint32_t schedulerTicks = 0;

static void idle_task_entry(void)
{
    for (;;)
    {
        asm volatile("hlt");
    }
}

static uintptr_t task_create_context(uint8_t* stack, void (*entry)(void))
{
    uintptr_t* sp = (uintptr_t*)(stack + STACK_SIZE);

    *(--sp) = (uintptr_t)task_exit;

    *(--sp) = 0x202;            // EFLAGS: IF = 1
    *(--sp) = 0x08;             // Kernel CS
    *(--sp) = (uintptr_t)entry; // EIP

    *(--sp) = 0;  // error code
    *(--sp) = 32; // interrupt number

    *(--sp) = 0; // EAX
    *(--sp) = 0; // ECX
    *(--sp) = 0; // EDX
    *(--sp) = 0; // EBX
    *(--sp) = 0; // original ESP
    *(--sp) = 0; // EBP
    *(--sp) = 0; // ESI
    *(--sp) = 0; // EDI

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

    nextTaskID = 1;

    // The kernel task
    tasks[0].state = TASK_RUNNING;

    // Start the idle task
    idleTask.id = (uint32_t)-1;
    idleTask.state = TASK_READY;
    idleTask.esp = task_create_context(idleStack, idle_task_entry);
    idleTask.next = NULL;

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

    uint32_t flags = irq_save();

    task_t* slot = NULL;
    for (uint32_t i = 1; i < MAX_TASKS; ++i)
    {
        if (tasks[i].state == TASK_TERMINATED)
        {
            slot = &tasks[i];
            break;
        }
    }

    if (!slot)
    {
        irq_restore(flags);
        return -1;
    }

    // Only assign a fresh ID for a brand new task
    if (slot->id == 0)
    {
        slot->id = nextTaskID++;
    }

    if (slot->id - 1 >= MAX_USER_TASKS)
    {
        irq_restore(flags);
        return -1;
    }

    slot->state = TASK_READY;
    slot->esp = task_create_context(taskStacks[slot->id - 1], entry);

    irq_restore(flags);

    return (int)slot->id;
}

void task_yield(void)
{
    asm volatile("int $49");
}

void task_exit(void)
{
    asm volatile("int $50");

    for (;;)
    {
        asm volatile("hlt");
    }
}

void task_block(void)
{
    curTask->state = TASK_BLOCKED;
    task_yield();
}

int task_unblock(uint32_t id)
{
    uint32_t flags = irq_save();

    for (uint32_t i = 1; i < MAX_TASKS; ++i)
    {
        if (tasks[i].id == id && tasks[i].state == TASK_BLOCKED)
        {
            tasks[i].state = TASK_READY;
            irq_restore(flags);
            return 0;
        }
    }

    irq_restore(flags);
}

struct registers* scheduler_schedule(struct registers* regs)
{
    if (!curTask || !regs)
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

            return (struct registers*)curTask->esp;
        }
    }

    // If the current task is ready, it is safe to continue to run it
    if (curTask->state == TASK_READY)
    {
        curTask->state = TASK_RUNNING;
        schedulerTicks = 0;

        return (struct registers*)curTask->esp;
    }

    curTask = &idleTask;
    curTask->state = TASK_RUNNING;
    schedulerTicks = 0;
}

struct registers* scheduler_tick(struct registers* regs)
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

struct registers* scheduler_exit(struct registers* regs)
{
    if (!curTask || !regs)
    {
        return regs;
    }

    // Mark the task terminated before the scheduler
    curTask->state = TASK_TERMINATED;

    return scheduler_schedule(regs);
}
