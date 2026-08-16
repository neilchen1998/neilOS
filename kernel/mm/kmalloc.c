#include "kmalloc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ALIGN8(size) (((size) + 7) & ~(size_t)7) // ~(size_t)7 clears the lowest 3 bits

#define HEAP_SIZE (1024 * 1024) // 1 MB

typedef struct block
{
    size_t        size; // payload bytes (excluding header)
    bool          free;
    struct block* next;
} block_t;

#define HEADER_SIZE ALIGN8(sizeof(block_t))

_Static_assert(HEADER_SIZE % 8 == 0, "HEADER_SIZE must be a multiple of 8");

// static uint8_t heap[HEAP_SIZE] __attribute__((aligned(16)));

extern uint8_t __heap_start[];

static uint8_t* const heap = __heap_start;

static block_t* head = NULL;

void kmalloc_init(void)
{
    head = (block_t*)heap;
    head->size = HEAP_SIZE - HEADER_SIZE;
    head->free = true;
    head->next = NULL;
}

void* kmalloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }

    // Round up to 8-byte alignment
    size = ALIGN8(size);

    // Find the next free space in the memory
    for (block_t* ptr = head; ptr != NULL; ptr = ptr->next)
    {
        // Keep on searching if the block is not free or the size is too small
        if (!ptr->free || ptr->size < size)
        {
            continue;
        }

        // Check if the remaining size is enough
        if (ptr->size >= (size + HEADER_SIZE + 8))
        {
            // Silce up the remaining space into two blocks: ptr and split
            // ptr block will be used by the caller and split will point to the next available space
            block_t* split = (block_t*)((uint8_t*)ptr + HEADER_SIZE + size);
            split->size = ptr->size - size - HEADER_SIZE;
            split->free = true;
            split->next = ptr->next;

            ptr->size = size;
            ptr->next = split;
        }

        ptr->free = false;

        return (uint8_t*)ptr + HEADER_SIZE;
    }

    return NULL;
}

void kfree(void* ptr)
{
    if (!ptr)
    {
        return;
    }

    block_t* b = (block_t*)((uint8_t*)ptr - HEADER_SIZE);
    b->free = true;

    for (block_t* cur = head; cur != NULL; cur = cur->next)
    {
        while (cur->free && cur->next != NULL && cur->next->free)
        {
            cur->size += HEADER_SIZE + cur->next->size;
            cur->next = cur->next->next;
        }
    }
}
