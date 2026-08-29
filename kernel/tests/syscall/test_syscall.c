#include "test_syscall.h"

#include <stddef.h>
#include <stdint.h>

#include "arch/x86/idt.h"
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
    test_assert("Reject NULL register state", result != NULL);

    fvga_write("\n=== SYSCALL TEST COMPLETE ===\n");

    return true;
}
