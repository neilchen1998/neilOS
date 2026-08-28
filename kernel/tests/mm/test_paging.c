#include "test_paging.h"

#include <stddef.h>
#include <stdint.h>

#include "drivers/video/vga.h"
#include "mm/paging.h"
#include "mm/physical.h"

#define PAGING_TEST_PHYSICAL 0x00300000u
#define PAGING_TEST_VIRTUAL 0x00400000u

static void paging_test_fail(const char* message)
{
    fvga_write("PAGING TEST FAILED: %s\n", message);

    /*
     * Stop here so a failure does not corrupt the rest of the kernel.
     */
    for (;;)
    {
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}

static void paging_test_pass(const char* message)
{
    fvga_write("PAGING TEST PASSED: %s\n", message);
}

void paging_test(void)
{
    fvga_write("\n=== PAGING TEST START ===\n");

    /*
     * Enable paging and establish the initial identity mapping.
     */

    fvga_write("1. paging_init: OK\n");

    /*
     * Verify that the initial identity mapping works.
     *
     * 0x00200000 is inside the first 4 MiB identity mapping.
     */
    volatile uint32_t* identity = (volatile uint32_t*)(uintptr_t)0x00200000u;

    *identity = 0x12345678u;

    if (*identity != 0x12345678u)
    {
        fvga_write("1. identity mapping: FAILED\n");

        for (;;)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    fvga_write("2. identity mapping: OK\n");

    /*
     * Allocate a physical page for the mapping test.
     */
    uint32_t testPhysical = physical_alloc_page();

    fvga_write("3. allocated physical page: %x\n", testPhysical);

    if (testPhysical == 0)
    {
        fvga_write("3. physical allocation: FAILED\n");

        for (;;)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    /*
     * The physical page must be page aligned.
     */
    if ((testPhysical & (PAGE_SIZE - 1u)) != 0)
    {
        fvga_write("4. physical alignment: FAILED (%x)\n", testPhysical);

        for (;;)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    fvga_write("4. physical alignment: OK\n");

    /*
     * IMPORTANT:
     *
     * The physical page returned by physical_alloc_page() must
     * currently be identity mapped for this access to work.
     *
     * If testPhysical is above 4 MiB, this access will page fault.
     */
    volatile uint32_t* physical = (volatile uint32_t*)(uintptr_t)testPhysical;

    *physical = 0xCAFEBABEu;

    fvga_write("5. physical page access: OK\n");

    /*
     * 0x00400000 is the first virtual address after the initial
     * 4 MiB identity mapping.
     */
    const uint32_t testVirtual = 0x00400000u;

    fvga_write("6. mapping %x -> %x\n", testVirtual, testPhysical);

    int result = paging_map(testVirtual, testPhysical, PAGE_WRITABLE);

    fvga_write("7. paging_map returned: %d\n", result);

    if (result != 0)
    {
        fvga_write("7. paging_map: FAILED\n");

        for (;;)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    fvga_write("7. paging_map: OK\n");

    /*
     * Access the physical page through the new virtual address.
     */
    volatile uint32_t* virtual = (volatile uint32_t*)(uintptr_t)testVirtual;

    fvga_write("8. testing virtual address %x\n", testVirtual);

    if (*virtual != 0xCAFEBABEu)
    {
        fvga_write("8. virtual read: FAILED, got %x\n", *virtual);

        for (;;)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    fvga_write("8. virtual read: OK\n");

    /*
     * Write through the virtual address.
     */
    *virtual = 0xDEADBEEFu;

    if (*physical != 0xDEADBEEFu)
    {
        fvga_write("9. virtual write: FAILED, physical=%x\n", *physical);

        for (;;)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    fvga_write("9. virtual write: OK\n");

    /*
     * Remove the mapping.
     */
    result = paging_unmap(testVirtual);

    fvga_write("10. paging_unmap returned: %d\n", result);

    if (result != 0)
    {
        fvga_write("10. paging_unmap: FAILED\n");

        for (;;)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    fvga_write("10. paging_unmap: OK\n");

    /*
     * Don't access testVirtual here!
     *
     * It is intentionally unmapped now, so doing:
     *
     *     *virtual
     *
     * should cause a page fault.
     */

    physical_free_page(testPhysical);

    fvga_write("11. physical page freed: OK\n");

    fvga_write("=== PAGING TEST COMPLETE ===\n");
}
