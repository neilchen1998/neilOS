#ifndef KERNEL_MM_KMALLOC_H
#define KERNEL_MM_KMALLOC_H

#include <stddef.h>

/// @brief Initializes the kernel memory allocator.
void kmalloc_init(void);

/// @brief Allocates a block of kernel memory.
///
/// @param size Number of bytes to allocate.
/// @return Pointer to the allocated memory on success, NULL on failure.
void* kmalloc(size_t size);

/// @brief Frees a previously allocated block of kernel memory.
///
/// @param ptr Pointer to the memory block to free. NULL is ignored.
void kfree(void* ptr);

#endif  // KERNEL_MM_KMALLOC_H
