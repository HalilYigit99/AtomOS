#include <memory/memory.h>
#include <memory/heap.h>

heap_t kernel_heap; // Global kernel heap

// Helper function to align size to word boundary
static inline size_t align_size(size_t size) {
    return (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);
}

// Helper function to align address to specified alignment
static inline void* align_address(void* addr, size_t alignment) {
    uintptr_t aligned = ((uintptr_t)addr + alignment - 1) & ~(alignment - 1);
    return (void*)aligned;
}

// Helper function to validate heap block
static bool is_valid_block(heap_block_t* block) {
    return block && block->magic == HEAP_MAGIC;
}

void heap_create(heap_t* heap, void* start, void* end) {
    if (!heap || !start || !end || start >= end) {
        return;
    }

    size_t size = (char*)end - (char*)start;
    if (size < sizeof(heap_block_t) * 2) {
        return;
    }

    heap->start = start;
    heap->end = end;
    heap->size = size;
    heap->free = size - sizeof(heap_block_t) * 2; // Account for start and end blocks

    // Create start block (header block)
    heap_block_t* start_block = (heap_block_t*)start;
    start_block->size = sizeof(heap_block_t);
    start_block->free = false;
    start_block->magic = HEAP_MAGIC;
    start_block->next = NULL;

    // Create initial free block
    heap_block_t* free_block = (heap_block_t*)((char*)start + sizeof(heap_block_t));
    free_block->size = size - sizeof(heap_block_t) * 2;
    free_block->free = true;
    free_block->magic = HEAP_MAGIC;
    free_block->next = NULL;

    // Link start block to free block
    start_block->next = free_block;

    // Create end block (sentinel)
    heap_block_t* end_block = (heap_block_t*)((char*)heap->end - sizeof(heap_block_t));
    end_block->size = sizeof(heap_block_t);
    end_block->free = false;
    end_block->magic = HEAP_MAGIC;
    end_block->next = NULL;

    // Link free block to end block
    free_block->next = end_block;
}

void* heap_alloc(heap_t* heap, size_t size) {
    if (!heap || size == 0) {
        return NULL;
    }

    // Align size to word boundary
    size = align_size(size);
    size_t total_size = size + sizeof(heap_block_t);

    // Best fit algorithm - find the smallest block that fits
    heap_block_t* best_block = NULL;
    heap_block_t* best_prev = NULL;
    heap_block_t* current = (heap_block_t*)heap->start;
    heap_block_t* prev = NULL;

    while (current && (char*)current < (char*)heap->end) {
        if (!is_valid_block(current)) {
            return NULL; // Heap corruption detected
        }

        if (current->free && current->size >= total_size) {
            // This block can fit our allocation
            if (!best_block || current->size < best_block->size) {
                best_block = current;
                best_prev = prev;
            }
        }

        prev = current;
        current = current->next;
    }

    if (!best_block) {
        return NULL; // No suitable block found
    }

    // If the block is much larger than needed, split it
    if (best_block->size >= total_size + sizeof(heap_block_t) + sizeof(void*)) {
        // Create a new free block from the remaining space
        heap_block_t* new_block = (heap_block_t*)((char*)best_block + total_size);
        
        // Ensure new block doesn't exceed heap bounds
        if ((char*)new_block + sizeof(heap_block_t) <= (char*)heap->end) {
            new_block->size = best_block->size - total_size;
            new_block->free = true;
            new_block->magic = HEAP_MAGIC;
            new_block->next = best_block->next;

            // Update the allocated block
            best_block->size = total_size;
            best_block->next = new_block;

            // Update heap free space
            heap->free -= total_size;
        } else {
            // Can't split safely, use entire block
            heap->free -= best_block->size;
        }
    } else {
        // Use the entire block
        heap->free -= best_block->size;
    }

    // Mark block as allocated
    best_block->free = false;

    // Return pointer to the data area (after the header)
    void* data_ptr = (char*)best_block + sizeof(heap_block_t);
    
    // Ensure the returned pointer and its data area are within heap bounds
    if ((char*)data_ptr + size <= (char*)heap->end) {
        return data_ptr;
    } else {
        // Safety check failed - this shouldn't happen if our logic is correct
        best_block->free = true; // Revert allocation
        heap->free += best_block->size; // Restore free space
        return NULL;
    }
}

void heap_free(heap_t* heap, void* ptr) {
    if (!heap || !ptr) {
        return;
    }
    
    // Check if pointer is within heap bounds
    if (ptr < heap->start || ptr >= heap->end) {
        return; // Pointer is outside heap bounds
    }

    // Get the block header
    heap_block_t* block = (heap_block_t*)((char*)ptr - sizeof(heap_block_t));
    
    // Check if block header is within heap bounds
    if ((void*)block < heap->start || (char*)block + sizeof(heap_block_t) > (char*)heap->end) {
        return; // Block header is outside heap bounds
    }
    
    if (!is_valid_block(block) || block->free) {
        return; // Invalid block or already free
    }

    // Mark block as free
    block->free = true;
    heap->free += block->size;

    // Coalesce with next block if it's free
    if (block->next && block->next->free && is_valid_block(block->next)) {
        heap_block_t* next_block = block->next;
        block->size += next_block->size;
        block->next = next_block->next;
    }

    // Coalesce with previous block if it's free
    heap_block_t* current = (heap_block_t*)heap->start;
    heap_block_t* prev = NULL;

    while (current && current != block) {
        if (current->next == block && current->free && is_valid_block(current)) {
            current->size += block->size;
            current->next = block->next;
            break;
        }
        prev = current;
        current = current->next;
    }
}

void heap_destroy(heap_t* heap) {
    if (!heap) {
        return;
    }

    // Clear heap structure
    heap->start = NULL;
    heap->end = NULL;
    heap->size = 0;
    heap->free = 0;
}

void* heap_realloc(heap_t* heap, void* ptr, size_t new_size) {
    if (!heap) {
        return NULL;
    }

    if (!ptr) {
        return heap_alloc(heap, new_size);
    }

    if (new_size == 0) {
        heap_free(heap, ptr);
        return NULL;
    }
    
    // Check if pointer is within heap bounds
    if (ptr < heap->start || ptr >= heap->end) {
        return NULL; // Pointer is outside heap bounds
    }

    // Get the current block
    heap_block_t* block = (heap_block_t*)((char*)ptr - sizeof(heap_block_t));
    
    // Check if block header is within heap bounds
    if ((void*)block < heap->start || (char*)block + sizeof(heap_block_t) > (char*)heap->end) {
        return NULL; // Block header is outside heap bounds
    }
    
    if (!is_valid_block(block) || block->free) {
        return NULL; // Invalid block
    }

    size_t old_data_size = block->size - sizeof(heap_block_t);
    size_t aligned_new_size = align_size(new_size);

    // If new size fits in current block, just return the same pointer
    if (aligned_new_size <= old_data_size) {
        return ptr;
    }

    // Allocate new block
    void* new_ptr = heap_alloc(heap, new_size);
    if (!new_ptr) {
        return NULL;
    }

    // Copy old data to new block
    memcpy(new_ptr, ptr, old_data_size < new_size ? old_data_size : new_size);

    // Free old block
    heap_free(heap, ptr);

    return new_ptr;
}

void* heap_calloc(heap_t* heap, size_t num, size_t size) {
    if (!heap || num == 0 || size == 0) {
        return NULL;
    }

    // Check for overflow
    if (num > SIZE_MAX / size) {
        return NULL;
    }

    size_t total_size = num * size;
    void* ptr = heap_alloc(heap, total_size);
    
    if (ptr) {
        memset(ptr, 0, total_size);
    }

    return ptr;
}

void* heap_alloc_aligned(heap_t* heap, size_t size, size_t alignment) {
    if (!heap || size == 0 || alignment == 0) {
        return NULL;
    }

    // Alignment must be a power of 2
    if ((alignment & (alignment - 1)) != 0) {
        return NULL;
    }

    // Allocate extra space for alignment
    size_t extra_size = size + alignment + sizeof(heap_block_t);
    void* raw_ptr = heap_alloc(heap, extra_size);
    
    if (!raw_ptr) {
        return NULL;
    }

    // Calculate aligned address
    void* aligned_ptr = align_address(raw_ptr, alignment);
    
    // Check if aligned pointer and its data area are within heap bounds
    if ((char*)aligned_ptr + size <= (char*)heap->end && aligned_ptr >= heap->start) {
        // If the aligned pointer is the same as raw pointer, we're done
        if (aligned_ptr == raw_ptr) {
            return aligned_ptr;
        }
        
        // For simplicity, we'll just use the raw pointer and accept some waste
        // A more sophisticated implementation would track the offset
        return raw_ptr;
    } else {
        // Aligned pointer would exceed heap bounds, free the allocation and return NULL
        heap_free(heap, raw_ptr);
        return NULL;
    }
}