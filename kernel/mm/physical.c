#include "physical.h"

#include <stdint.h>

#define PAGE_SIZE 4096u

#define PHYSICAL_MEMORY_SIZE (64u * 1024u * 1024u)

#define PHYSICAL_PAGE_COUNT (PHYSICAL_MEMORY_SIZE / PAGE_SIZE)

#define BITMAP_WORD_COUNT ((PHYSICAL_PAGE_COUNT + 31u) / 32u)

static uint32_t page_bitmap[BITMAP_WORD_COUNT];

static uint32_t free_pages;

// These are defined by the linker
extern uint8_t __kernel_start[];
extern uint8_t __kernel_end[];

extern uint8_t __heap_start[];
extern uint8_t __heap_end[];

static void mark_used(uint32_t page)
{
    page_bitmap[page / 32u] |= 1u << (page % 32u);
}

static void mark_free(uint32_t page)
{
    page_bitmap[page / 32u] &= ~(1u << (page % 32u));
}

static int is_used(uint32_t page)
{
    return (page_bitmap[page / 32u] & (1u << (page % 32u))) != 0;
}

static void reserve_range(uint32_t start, uint32_t end)
{
    /*
     * Round the start down.
     */
    start &= ~(PAGE_SIZE - 1u);

    /*
     * Round the end up.
     */
    end = (end + PAGE_SIZE - 1u) & ~(PAGE_SIZE - 1u);

    for (uint32_t address = start; address < end; address += PAGE_SIZE)
    {
        uint32_t page = address / PAGE_SIZE;

        if (page >= PHYSICAL_PAGE_COUNT)
        {
            break;
        }

        if (!is_used(page))
        {
            mark_used(page);
            --free_pages;
        }
    }
}

void physical_init(void)
{
    // Mark all physical pages as unavailable
    for (uint32_t i = 0; i < BITMAP_WORD_COUNT; ++i)
    {
        page_bitmap[i] = 0xFFFFFFFFu;
    }

    free_pages = 0;

    // Assume the first 64 MiB is usable
    for (uint32_t page = 0; page < PHYSICAL_PAGE_COUNT; ++page)
    {
        mark_free(page);
        ++free_pages;
    }

    // Reserve physical page zero
    reserve_range(0x00000000u, 0x00001000u);

    // Reserve the low-memory area that includes BIOS, VGA, ROM, etc.
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

            --free_pages;

            return page * PAGE_SIZE;
        }
    }

    return 0;
}

void physical_free_page(uint32_t address)
{
    if ((address & (PAGE_SIZE - 1u)) != 0)
    {
        return;
    }

    uint32_t page = address / PAGE_SIZE;

    if (page >= PHYSICAL_PAGE_COUNT)
    {
        return;
    }

    /*
     * Don't allow reserved low memory to be freed.
     */
    if (address < 0x00100000u)
    {
        return;
    }

    /*
     * Don't free something that is already free.
     */
    if (!is_used(page))
    {
        return;
    }

    mark_free(page);
    ++free_pages;
}
