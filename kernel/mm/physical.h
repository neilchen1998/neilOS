#ifndef KERNEL_MM_PHYSICAL_H
#define KERNEL_MM_PHYSICAL_H

#include <stddef.h>
#include <stdint.h>

// @brief Initializes the physical page allocator.
//
// Marks all initially usable physical pages as free and reserves
// memory occupied by the kernel, kernel stack, heap, and other
// memory that must not be allocated.
void physical_init(void);

// @brief Allocates one physical memory page.
//
// Finds a free physical page, marks it as used, and returns its physical address.
//
// @return Physical address of the allocated 4 KiB page, or 0 if no free page is available.
uint32_t physical_alloc_page(void);

// @brief Frees a previously allocated physical memory page.
//
// Marks the specified page as available for future allocations.
//
// @param address Physical address of the page to free. The address must be 4 KiB aligned.
void physical_free_page(uint32_t address);

#endif // KERNEL_MM_PHYSICAL_H
