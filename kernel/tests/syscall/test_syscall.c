#include "test_syscall.h"

#include <stddef.h>
#include <stdint.h>

#include "arch/x86/idt.h"
#include "drivers/timer/pit.h"
#include "drivers/video/vga.h"
#include "stdbool.h"
#include "syscall/syscall.h"
#include "test_assert.h"

bool syscall_test(void)
{
    struct registers regs = {0};
    struct registers* result = NULL;

    fvga_write("\n=== SYSCALL TEST START ===\n");

    // Case (i): null register
    result = syscall_dispatch(NULL);
    EXPECT_FALSE("Reject NULL register state", result);

    // Case (iv): tick count
    uint32_t before = pit_get_ticks();
    regs.eax = SYS_GET_TICKS;
    result = syscall_dispatch(&regs);
    uint32_t after = pit_get_ticks();

    EXPECT_TRUE("syscall_dispatch returns regs", result == &regs);
    EXPECT_GE("SYS_GET_TICKS returns a valid tick count", regs.eax, before);
    EXPECT_LE("SYS_GET_TICKS returns a valid tick count", regs.eax, after);

    fvga_write("\n=== SYSCALL TEST COMPLETE ===\n");

    return true;
}
