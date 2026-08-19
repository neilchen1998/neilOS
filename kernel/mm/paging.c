#include "paging.h"

#include <stdint.h>
#include <sys/cdefs.h>

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
    for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; ++i)
    {
        pageDirectory[i] = 0;

        // Create an identity mapping
        // By ORing PAGE_PRESENT, it makes the page as valid
        // By ORing PAGE_WRITABLE, it makes the page as writable
        firstPageTable[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITABLE;
    }
}

/// @brief
int paging_map(uint32_t virtualAddress, uint32_t physicalAddress, uint32_t flags)
{
}

/// @brief
int paging_unmap(uint32_t virtualAddress)
{
}
