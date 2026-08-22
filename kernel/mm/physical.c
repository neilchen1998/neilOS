#include "physical.h"

#include <stdint.h>

#define PAGE_SIZE 4096u

#define PHYSICAL_MEMORY_SIZE (64u * 1024u * 1024u)

#define PHYSICAL_PAGE_COUNT (PHYSICAL_MEMORY_SIZE / PAGE_SIZE)

#define BITMAP_WORD_COUNT ((PHYSICAL_PAGE_COUNT + 31u) / 32u)

// A compact array where each bit represents whether one page is free or used
// pageBitMap[0] -> pages  0-31
// pageBitMap[1] -> pages 32-63
static uint32_t pageBitMap[BITMAP_WORD_COUNT];

// The number of free pages
static uint32_t freePages;

// Start and end address of kernel defined by the linker script
extern uint8_t __kernel_start[];
extern uint8_t __kernel_end[];

// Start and end address of heap defined by the linker script
extern uint8_t __heap_start[];
extern uint8_t __heap_end[];

// @brief Marks a page as used in the page bitmap.
//
// Each uint32_t in page_bitmap stores the allocation state
// of 32 pages. The page number determines both which bitmap
// entry contains the page's bit and which bit within that entry.
//
// @param page Page number to mark as used.
static void mark_used(uint32_t page)
{
    pageBitMap[page / 32u] |= 1u << (page % 32u);
}

// @brief Marks a page as free in the page bitmap.
//
// Finds the bitmap entry containing the page's bit and clears
// that bit without modifying the state of any other page.
//
// @param page Page number to mark as free.
static void mark_free(uint32_t page)
{
    pageBitMap[page / 32u] &= ~(1u << (page % 32u));
}

// @brief Checks whether a page is currently marked as used.
//
// Finds the bit corresponding to the given page and tests
// whether that bit is set.
//
// @param page Page number to check.
// @return Non-zero if the page is used, or 0 if the page is free.
static int is_used(uint32_t page)
{
    return (pageBitMap[page / 32u] & (1u << (page % 32u))) != 0;
}

static void reserve_range(uint32_t start, uint32_t end)
{
    // The allocator works in whole pages, not in arbitary byte ranges
    // Therefore, we need to find a whole page
    start &= ~(PAGE_SIZE - 1u);                       // rounds the start down
    end = (end + PAGE_SIZE - 1u) & ~(PAGE_SIZE - 1u); // rounds the end up

    for (uint32_t address = start; address < end; address += PAGE_SIZE)
    {
        uint32_t page = address / PAGE_SIZE;

        // Stop when we reach the end of the pages
        if (page >= PHYSICAL_PAGE_COUNT)
        {
            break;
        }

        // Only mark used if it is used
        if (!is_used(page))
        {
            mark_used(page);
            --freePages;
        }
    }
}

void physical_init(void)
{
    // Mark all physical pages as unavailable
    for (uint32_t i = 0; i < BITMAP_WORD_COUNT; ++i)
    {
        pageBitMap[i] = 0xFFFFFFFFu;
    }

    freePages = 0;

    // Assume the first 64 MiB is usable
    for (uint32_t page = 0; page < PHYSICAL_PAGE_COUNT; ++page)
    {
        mark_free(page);
        ++freePages;
    }

    // Reserve physical page zero
    reserve_range(0x00000000u, 0x00001000u);

    // Reserve the real mode address space (< 1 MiB) that includes BIOS, VGA, ROM, etc.
    reserve_range(0x00000000u, 0x00100000u);

    // Reserve the kernel area
    reserve_range((uint32_t)(uintptr_t)__kernel_start, (uint32_t)(uintptr_t)__kernel_end);

    // Reserve the kmalloc heap area
    reserve_range((uint32_t)(uintptr_t)__heap_start, (uint32_t)(uintptr_t)__heap_end);

    // Reserve the kernel stack
    reserve_range(0x00098000u, 0x000A0000u);
}

uint32_t physical_alloc_page(void)
{
    for (uint32_t page = 256; page < PHYSICAL_PAGE_COUNT; ++page)
    {
        if (!is_used(page))
        {
            mark_used(page);

            --freePages;

            return page * PAGE_SIZE;
        }
    }

    return 0;
}

void physical_free_page(uint32_t address)
{
    // Check if address is aligned to the beginning of a memory page
    // The address needs to be the start of a page, if not, then do not free it
    if ((address & (PAGE_SIZE - 1u)) != 0)
    {
        return;
    }

    uint32_t page = address / PAGE_SIZE;

    if (page >= PHYSICAL_PAGE_COUNT)
    {
        return;
    }

    // Skip if the address is in the low memory
    if (address < 0x00100000u)
    {
        return;
    }

    // Skip if the page is not used
    if (!is_used(page))
    {
        return;
    }

    mark_free(page);
    ++freePages;
}
