#include "paging.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "drivers/video/vga.h"
#include "physical.h"

#define PAGE_TABLE_ENTRIES 1024u
#define PAGE_DIRECTORY_ENTRIES 1024u
#define PAGE_ADDRESS_MASK 0xFFFFF000u

#define PAGE_DIRECTORY_INDEX(address) (((address) >> 22) & 0x3FFu)

#define PAGE_TABLE_INDEX(address) (((address) >> 12) & 0x3FFu)

#define PAGE_OFFSET(address) ((address) & 0xFFFu)

static uint32_t pageDirectory[PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint32_t firstPageTable[PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

// @brief Converts a physical address to a kernel-accessible virtual address.
//
// @param physicalAddress The physical address.
// @return The virtual address.
static inline uint32_t* physical_to_virtual(uint32_t physicalAddress)
{
    return (uint32_t*)(uintptr_t)physicalAddress;
}

// @brief Checks if the address is page aligned.
//
// @param address The address to be validated
// @return TRUE if the page address is page-aligned and below NIT_ADDRESS_SPACE, FALSE otherwise.
inline static bool page_address_valid(uint32_t address)
{
    return (address & (PAGE_SIZE - 1u)) == 0;
}

// @brief Flushes the virtual address from the TLB (translation lookaside buffer).
//
// @param virtualAddress The virtual address whose TLB entry should be invalidated.
static inline void paging_invalidate_page(uint32_t virtualAddress)
{
    asm volatile("invlpg (%0)" : : "r"((void*)(uintptr_t)virtualAddress) : "memory");
}

// @brief Reads the current value of the CR3 control register.
//
// CR3 contains the physical address of the current page directory
// or page-table hierarchy used for address translation.
//
// @return Current value of the CR3 register.
static inline uint32_t paging_get_cr3(void)
{
    uint32_t value;

    asm volatile("mov %%cr3, %0" : "=r"(value) : : "memory");

    return value;
}

// @brief Loads the specified physical address into the CR3 control register.
//
// @param address Physical address of the page directory or page-table hierarchy to load into CR3.
static inline void paging_load_cr3(uint32_t address)
{

    asm volatile("mov %0, %%cr3" : : "r"(address) : "memory");
}

// @brief Enables paging in CR0.
//
// Turn on paging by getting CR0 from the CPU, flipping PG (page-enabling) bit to 1, and writing it back to the CPU.
static inline void paging_enable(void)
{
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u;
    asm volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}

// @brief Disables paging in CR0.
//
static inline void paging_disable(void)
{
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~0x80000000u;
    asm volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}

static uint32_t allocate_page_table(void)
{
    uint32_t physicalAddress;
    uint32_t* pageTable;

    physicalAddress = physical_alloc_page();

    if (physicalAddress == 0)
    {
        return 0;
    }

    pageTable = physical_to_virtual(physicalAddress);

    for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; ++i)
    {
        pageTable[i] = 0;
    }

    return physicalAddress;
}

void paging_init(void)
{
    // Clear the page directory
    for (uint32_t i = 0; i < PAGE_DIRECTORY_ENTRIES; ++i)
    {
        pageDirectory[i] = 0;
    }

    // Create the first page table
    for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; ++i)
    {
        // By ORing PAGE_PRESENT, it makes the page as valid
        // By ORing PAGE_WRITABLE, it makes the page as writable
        firstPageTable[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITABLE;
    }

    pageDirectory[0] = (uint32_t)(uintptr_t)firstPageTable | PAGE_PRESENT | PAGE_WRITABLE;

    // Load the physical address of the page directory into the CPU's CR3 register
    uint32_t directory = (uint32_t)(uintptr_t)pageDirectory;
    paging_load_cr3(directory);

    paging_enable();
}

int paging_map(uint32_t virtualAddress, uint32_t physicalAddress, uint32_t flags)
{
    fterminal_write("paging_map: VA=%x PA=%x\n", virtualAddress, physicalAddress);

    fterminal_write("paging_map: VA valid=%d PA valid=%d\n", page_address_valid(virtualAddress), page_address_valid(physicalAddress));

    if (!page_address_valid(virtualAddress))
    {
        fterminal_write("paging_map: INVALID VIRTUAL\n");
        return -1;
    }

    if (!page_address_valid(physicalAddress))
    {
        fterminal_write("paging_map: INVALID PHYSICAL\n");
        return -1;
    }

    // Check if both addresses are page-aligned
    if (!page_address_valid(virtualAddress) || !page_address_valid(physicalAddress))
    {
        return -1;
    }

    uint32_t direcotryIdx = PAGE_DIRECTORY_INDEX(virtualAddress);
    uint32_t tableIdx = PAGE_TABLE_INDEX(virtualAddress);
    uint32_t directoryEntry = pageDirectory[direcotryIdx];

    uint32_t* pageTable;

    // Check if the required page table exists
    if (!(directoryEntry & PAGE_PRESENT))
    {
        uint32_t tablePhysicalAddress = allocate_page_table();

        if (tablePhysicalAddress == 0)
        {
            return -2;
        }

        pageDirectory[direcotryIdx] = (tablePhysicalAddress & PAGE_ADDRESS_MASK) | PAGE_PRESENT | PAGE_WRITABLE;

        pageTable = physical_to_virtual(tablePhysicalAddress);
    }
    else
    {
        uint32_t tablePhysicalAddress = (directoryEntry & PAGE_ADDRESS_MASK);
        pageTable = physical_to_virtual(tablePhysicalAddress);
    }

    // Check if an existing mapping exists
    if (pageTable[tableIdx] & PAGE_PRESENT)
    {
        return -3;
    }

    // Install the physical-to-virtual mapping
    pageTable[tableIdx] = (physicalAddress & PAGE_ADDRESS_MASK) | (flags & ~PAGE_ADDRESS_MASK);

    pageTable[tableIdx] |= PAGE_PRESENT;

    paging_invalidate_page(virtualAddress);

    return 0;
}

int paging_unmap(uint32_t virtualAddress)
{
    if (!page_address_valid(virtualAddress))
    {
        return -1;
    }

    uint32_t directoryIndex = PAGE_DIRECTORY_INDEX(virtualAddress);

    uint32_t tableIndex = PAGE_TABLE_INDEX(virtualAddress);

    uint32_t directoryEntry = pageDirectory[directoryIndex];

    if (!(directoryEntry & PAGE_PRESENT))
    {
        return -2;
    }

    uint32_t tablePhysicalAddress = directoryEntry & PAGE_ADDRESS_MASK;

    uint32_t* pageTable = physical_to_virtual(tablePhysicalAddress);

    if (!(pageTable[tableIndex] & PAGE_PRESENT))
        return -3;

    // Remove mapping
    pageTable[tableIndex] = 0;

    paging_invalidate_page(virtualAddress);

    return 0;
}

#define PAGING_TEST_PHYSICAL 0x00300000u
#define PAGING_TEST_VIRTUAL 0x00400000u

static void paging_test_fail(const char* message)
{
    fterminal_write("PAGING TEST FAILED: %s\n", message);

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
    fterminal_write("PAGING TEST PASSED: %s\n", message);
}

void paging_test(void)
{
    fterminal_write("\n=== PAGING TEST START ===\n");

    /*
     * Enable paging and establish the initial identity mapping.
     */
    paging_init();

    fterminal_write("1. paging_init: OK\n");

    /*
     * Verify that the initial identity mapping works.
     *
     * 0x00200000 is inside the first 4 MiB identity mapping.
     */
    volatile uint32_t* identity = (volatile uint32_t*)(uintptr_t)0x00200000u;

    *identity = 0x12345678u;

    if (*identity != 0x12345678u)
    {
        fterminal_write("1. identity mapping: FAILED\n");

        for (;;)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    fterminal_write("2. identity mapping: OK\n");

    /*
     * Allocate a physical page for the mapping test.
     */
    uint32_t testPhysical = physical_alloc_page();

    fterminal_write("3. allocated physical page: %x\n", testPhysical);

    if (testPhysical == 0)
    {
        fterminal_write("3. physical allocation: FAILED\n");

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
        fterminal_write("4. physical alignment: FAILED (%x)\n", testPhysical);

        for (;;)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    fterminal_write("4. physical alignment: OK\n");

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

    fterminal_write("5. physical page access: OK\n");

    /*
     * 0x00400000 is the first virtual address after the initial
     * 4 MiB identity mapping.
     */
    const uint32_t testVirtual = 0x00400000u;

    fterminal_write("6. mapping %x -> %x\n", testVirtual, testPhysical);

    int result = paging_map(testVirtual, testPhysical, PAGE_WRITABLE);

    fterminal_write("7. paging_map returned: %d\n", result);

    if (result != 0)
    {
        fterminal_write("7. paging_map: FAILED\n");

        for (;;)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    fterminal_write("7. paging_map: OK\n");

    /*
     * Access the physical page through the new virtual address.
     */
    volatile uint32_t* virtual = (volatile uint32_t*)(uintptr_t)testVirtual;

    fterminal_write("8. testing virtual address %x\n", testVirtual);

    if (*virtual != 0xCAFEBABEu)
    {
        fterminal_write("8. virtual read: FAILED, got %x\n", *virtual);

        for (;;)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    fterminal_write("8. virtual read: OK\n");

    /*
     * Write through the virtual address.
     */
    *virtual = 0xDEADBEEFu;

    if (*physical != 0xDEADBEEFu)
    {
        fterminal_write("9. virtual write: FAILED, physical=%x\n", *physical);

        for (;;)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    fterminal_write("9. virtual write: OK\n");

    /*
     * Remove the mapping.
     */
    result = paging_unmap(testVirtual);

    fterminal_write("10. paging_unmap returned: %d\n", result);

    if (result != 0)
    {
        fterminal_write("10. paging_unmap: FAILED\n");

        for (;;)
        {
            __asm__ volatile("cli");
            __asm__ volatile("hlt");
        }
    }

    fterminal_write("10. paging_unmap: OK\n");

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

    fterminal_write("11. physical page freed: OK\n");

    fterminal_write("=== PAGING TEST COMPLETE ===\n");
}
