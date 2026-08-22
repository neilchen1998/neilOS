#ifndef KERNEL_MM_PAGING_H
#define KERNEL_MM_PAGING_H

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096u

// Page table entry flags
#define PAGE_PRESENT 0x1u  // indicates the page is valid and mapped
#define PAGE_WRITABLE 0x2u // allows writes to the page
#define PAGE_USER 0x4u     // allows user-mode code to access

/// @brief Initializes the address space and enables paging
void paging_init(void);

/// @brief Maps a virtual address to a physical address with the specified flags.
///
/// @param virtualAddress Virtual address to map.
/// @param physicalAddress Physical address to map to.
/// @param flags Page mapping flags controlling access and page attributes.
/// @return 0 on success; otherwise an error code.
int paging_map(uint32_t virtualAddress, uint32_t physicalAddress, uint32_t flags);

/// @brief Removes the page mapping for the specified virtual address.
///
/// @param virtualAddress Virtual address whose mapping should be removed.
/// @return 0 on success; otherwise an error code.
int paging_unmap(uint32_t virtualAddress);

#endif // KERNEL_MM_PAGING_H
