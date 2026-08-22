#include "paging.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "physical.h"

#define PAGE_TABLE_ENTRIES 1024u
#define PAGE_DIRECTORY_ENTRIES 1024u
#define PAGE_ADDRESS_MASK 0xFFFFF000u

#define PAGE_DIRECTORY_INDEX(address) (((address) >> 22) & 0x3FFu)

#define PAGE_TABLE_INDEX(address) (((address) >> 12) & 0x3FFu)

#define PAGE_OFFSET(address) ((address) & 0xFFFu)

#define INIT_ADDRESS_SPACE (PAGE_SIZE * PAGE_TABLE_ENTRIES)

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
    return ((address % PAGE_SIZE) == 0) && (address < INIT_ADDRESS_SPACE);
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
