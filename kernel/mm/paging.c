#include "paging.h"

#include <stddef.h>
#include <stdint.h>

#define PAGE_TABLE_ENTRIES 1024u
#define INIT_ADDRESS_SPACE (PAGE_SIZE * PAGE_TABLE_ENTRIES)

static uint32_t pageDirectory[PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint32_t firstPageTable[PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

inline static int page_address_valid(uint32_t address)
{
    return ((address % PAGE_SIZE) == 0) && (address < INIT_ADDRESS_SPACE);
}

void paging_init(void)
{
    // Create an identity mapping for the first page
    for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; ++i)
    {
        pageDirectory[i] = 0;

        // By ORing PAGE_PRESENT, it makes the page as valid
        // By ORing PAGE_WRITABLE, it makes the page as writable
        firstPageTable[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITABLE;
    }

    pageDirectory[0] = (uint32_t)(uintptr_t)firstPageTable | PAGE_PRESENT | PAGE_WRITABLE;

    // Load the physical address of the page directory into the CPU's CR3 register
    uint32_t directory = (uint32_t)(uintptr_t)pageDirectory;
    asm volatile("mov %0, %%cr3" : : "r"(directory) : "memory");

    // Turn on paging by getting CR0 from the CPU, flipping PG (page-enabling) bit to 1, and writing it back to the CPU
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u;
    asm volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}

/// @brief
int paging_map(uint32_t virtualAddress, uint32_t physicalAddress, uint32_t flags)
{
}

/// @brief
int paging_unmap(uint32_t virtualAddress)
{
}
