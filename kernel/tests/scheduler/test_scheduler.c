#include "test_scheduler.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "drivers/timer/pit.h"
#include "drivers/video/vga.h"
#include "scheduler/scheduler.h"
#include "test_output.h"

static volatile uint32_t cntA = 0;
static volatile uint32_t cntB = 0;

static volatile uint32_t runningTestCnt = 0;
static volatile uint32_t passedTestCnt = 0;
static volatile uint32_t failedTestCnt = 0;
static volatile uint32_t shortLiveTaskCnt = 0;

static volatile uint32_t yieldA = 0;
static volatile uint32_t yieldB = 0;

static volatile bool blockerStarted = false;
static volatile bool blockerWoke = false;

static size_t blockerID = (size_t)-1;

// @brief Prints the task that passes
//
// @param name The name of the task that passes
static void test_pass_task(const char* name)
{
    ++runningTestCnt;
    ++passedTestCnt;
    test_pass("[PASS] %s\n", name);
}

// @brief Prints the task that fails
//
// @param name The name of the task that fails
static void test_fail_task(const char* name)
{
    ++runningTestCnt;
    ++failedTestCnt;

    test_fail("[FAIL] %s\n", name);
}

// @brief A task that increases cntA and NEVER yields.
static void task_a(void)
{
    while (timer_ms() < 5000)
    {
        ++cntA;
    }

    task_exit();
}

// @brief A task that increases cntB and NEVER yields.
static void task_b(void)
{
    while (timer_ms() < 5000)
    {
        ++cntB;
    }

    task_exit();
}

// @brief A monitor task that monitors the value of cntA and cntB and checks if the CPU is preemptive scheduling.
static void monitor()
{
    uint64_t next = 0;

    while (timer_ms() < 5000)
    {
        if (timer_ms() >= next)
        {
            fterminal_write("A: %i B: %i\n", cntA, cntB);
            next = timer_ms() + 500;
        }

        task_yield();
    }

    // If either one of the counters is zero, that means this test fails
    if (cntA == 0 || cntB == 0)
    {
        test_fail("preemptive scheduling\n");
    }
    else
    {
        test_pass("preemptive scheduling\n");
    }

    task_exit();
}

// @brief A task that goes to sleep and is ready to be woken up by the waker task
static void blocker(void)
{
    blockerStarted = true;

    fterminal_write("blocker: sleeping...\n");

    task_block();

    // Can only be reached after calling task_unblock()
    blockerWoke = true;

    fterminal_write("blocker: woken!\n");

    task_exit();
}

// @brief Wakes up the blocker task by calling task_unblock() and checks if
// it can be woken up
static void waker(void)
{
    // Wait for blocker task to start
    while (!blockerStarted)
    {
        task_yield();
    }

    // Wait for a bit before waking the task
    uint64_t wakeTime = timer_ms() + 1000;

    while (timer_ms() < wakeTime)
    {
        task_yield();
    }

    int result = task_unblock(blockerID);

    fterminal_write("waker: unblock %i -> %i\n", blockerID, result);

    if (result < 0)
    {
        test_fail("task_unblock\n");
    }
    else
    {
        // Wait for the second section of the blocker task
        uint64_t endTime = timer_ms() + 100;

        while (timer_ms() < endTime)
        {
            task_yield();
        }

        if (blockerWoke)
        {
            test_pass("task block/unblock\n");
        }
        else
        {
            test_fail("task block/unblock\n");
        }
    }

    task_exit();
}

// @brief A short-lived task that immediately exits upon creation.
static void short_lived(void)
{
    shortLiveTaskCnt++;
    task_exit();
}

// Spawns 256 short-lived tasks
static void spawner(void)
{
    uint32_t failures = 0;

    for (uint32_t i = 0; i < 256; ++i)
    {
        // Check if we can create these short-lived tasks
        if (task_create(short_lived) < 0)
        {
            ++failures;
        }

        task_yield();
    }

    // Print the number of failures
    fterminal_write("spawner: create failures = %i\n", failures);

    uint64_t endTime = timer_ms() + 500;

    while (timer_ms() < endTime)
    {
        task_yield();
    }

    if (shortLiveTaskCnt == 0)
    {
        test_fail("short-lived task creation/execution\n");
    }
    else
    {
        test_pass("short-lived task creation/execution\n");
    }

    task_exit();
}

// @brief A task that increases yieldA and yields
static void yield_task_a(void)
{
    while (timer_ms() < 3000)
    {
        yieldA++;
        task_yield();
    }

    task_exit();
}

// @brief A task that increases yieldB and yields
static void yield_task_b(void)
{
    while (timer_ms() < 3000)
    {
        yieldB++;
        task_yield();
    }

    task_exit();
}

// @brief A monitor task that monitors the value of yieldA and yieldB and checks if these tasks can yield
static void yield_test_monitor(void)
{
    while (timer_ms() < 3500)
    {
        task_yield();
    }

    if (yieldA == 0 || yieldB == 0)
    {
        test_fail("task_yield\n");
    }
    else
    {
        test_pass("task_yield\n");
    }

    task_exit();
}

// @brief Prints the final test results.
static void test_result(void)
{
    // Wait until all tests should have finished
    while (timer_ms() < 6000)
    {
        task_yield();
    }

    fterminal_write("\n");
    fterminal_write("==============================\n");
    fterminal_write("Scheduler test results\n");
    fterminal_write("==============================\n");

    fterminal_write("Tests run:    %i\n", runningTestCnt);
    fterminal_write("Tests passed: %i\n", passedTestCnt);
    fterminal_write("Tests failed: %i\n", failedTestCnt);

    if (failedTestCnt == 0)
    {
        test_pass("\n=== ALL TESTS PASSED ===\n");
    }
    else
    {

        test_fail("\n=== TESTS FAILED ===\n");
    }

    task_exit();
}

void scheduler_test(void)
{
    fterminal_write("\n");
    fterminal_write("==============================\n");
    fterminal_write("Starting scheduler tests...\n");
    fterminal_write("==============================\n");

    // Blocking/unblocking test
    blockerID = task_create(blocker);

    if (blockerID == (size_t)-1)
    {
        test_fail("create blocker task\n");
    }

    if (task_create(waker) < 0)
    {
        test_fail("create waker task\n");
    }

    // Preemption test
    if (task_create(monitor) < 0)
    {
        test_fail("create preemption monitor\n");
    }

    if (task_create(task_a) < 0)
    {
        test_fail("create task A\n");
    }

    if (task_create(task_b) < 0)
    {
        test_fail("create task B\n");
    }

    // Task creation / termination test
    if (task_create(spawner) < 0)
    {
        test_fail("create spawner task\n");
    }

    // Cooperative yield test
    if (task_create(yield_task_a) < 0)
    {
        test_fail("create yield task A\n");
    }

    if (task_create(yield_task_b) < 0)
    {
        test_fail("create yield task B\n");
    }

    if (task_create(yield_test_monitor) < 0)
    {
        test_fail("create yield monitor\n");
    }

    // Print the final result after the tests have had time to run
    if (task_create(test_result) < 0)
    {
        // Test result task can not be created so we just print failed
        test_fail("\n=== FAILED TO CREATE TEST RESULT TASK ===\n");
    }
}
