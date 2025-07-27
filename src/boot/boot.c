#include <stddef.h>
#include <stdint.h>

#include <boot/multiboot2.h>

#include <memory/memory.h>
#include <memory/heap.h>

#define KHEAP_MIN_SIZE 0x1000000 // 16 MB for kernel heap
#define KERNEL_START 0x100000 // 1 MB start address for kernel heap
#define MULTIBOOT_MEMORY_AVAILABLE 1 // Available memory type in memory map

extern char __kernel_end; // End of kernel binary

// Best fit algorithm to find suitable memory region for kernel heap
static struct multiboot_mmap_entry* find_best_fit_memory(size_t required_size, uint64_t* heap_start, uint64_t* heap_end) {
    if (!mb2_mmap) {
        return NULL;
    }
    
    struct multiboot_mmap_entry* best_entry = NULL;
    uint64_t best_size = UINT64_MAX;
    uint64_t kernel_start_addr = (uint64_t)KERNEL_START;
    uint64_t kernel_end_addr = (uint64_t)&__kernel_end;
    
    // Iterate through memory map entries
    size_t entry_count = (mb2_mmap->size - sizeof(struct multiboot_tag_mmap)) / mb2_mmap->entry_size;
    
    for (size_t i = 0; i < entry_count; i++) {
        struct multiboot_mmap_entry* entry = &mb2_mmap->entries[i];
        
        // Only consider available memory regions
        if (entry->type != MULTIBOOT_MEMORY_AVAILABLE) {
            continue;
        }
        
        uint64_t region_start = entry->addr;
        uint64_t region_end = entry->addr + entry->len;
        uint64_t available_start = region_start;
        uint64_t available_end = region_end;
        
        // Check if this region overlaps with kernel
        if (region_start < kernel_end_addr && region_end > kernel_start_addr) {
            // Region overlaps with kernel, calculate non-overlapping parts
            if (region_start < kernel_start_addr) {
                // Check space before kernel
                uint64_t before_kernel_size = kernel_start_addr - region_start;
                if (before_kernel_size >= required_size) {
                    // Space before kernel is sufficient
                    available_start = region_start;
                    available_end = kernel_start_addr;
                } else {
                    // Check space after kernel
                    if (kernel_end_addr < region_end) {
                        uint64_t after_kernel_size = region_end - kernel_end_addr;
                        if (after_kernel_size >= required_size) {
                            available_start = kernel_end_addr;
                            available_end = region_end;
                        } else {
                            continue; // Not enough space
                        }
                    } else {
                        continue; // Not enough space
                    }
                }
            } else {
                // Region starts after kernel start but before kernel end
                if (kernel_end_addr < region_end) {
                    uint64_t after_kernel_size = region_end - kernel_end_addr;
                    if (after_kernel_size >= required_size) {
                        available_start = kernel_end_addr;
                        available_end = region_end;
                    } else {
                        continue; // Not enough space
                    }
                } else {
                    continue; // Not enough space
                }
            }
        }
        
        uint64_t available_size = available_end - available_start;
        
        // Check if this region has enough space
        if (available_size >= required_size) {
            // Best fit: choose the smallest sufficient region
            if (available_size < best_size) {
                best_size = available_size;
                best_entry = entry;
                *heap_start = available_start;
                *heap_end = available_start + required_size; // Only allocate what we need
            }
        }
    }
    
    return best_entry;
}

void __kernel_setup() {
    // Ensure multiboot2 information is parsed first
    if (!mb2_mmap) {
        // Memory map is not available - cannot setup heap
        return;
    }
    
    // Look for the kernel heap in the memory map using best fit algorithm
    uint64_t heap_start_addr = 0;
    uint64_t heap_end_addr = 0;
    
    // Find the best memory region for kernel heap
    struct multiboot_mmap_entry* heap_region = find_best_fit_memory(KHEAP_MIN_SIZE, &heap_start_addr, &heap_end_addr);
    
    if (!heap_region) {
        // No suitable memory region found - this is a critical error
        // In a real OS, you might want to panic here
        return;
    }
    
    // Ensure heap addresses are properly aligned (4KB alignment)
    heap_start_addr = (heap_start_addr + 0xFFF) & ~0xFFF;
    heap_end_addr = (heap_end_addr + 0xFFF) & ~0xFFF;
    
    // Ensure we still have enough space after alignment
    if (heap_end_addr - heap_start_addr < KHEAP_MIN_SIZE) {
        return;
    }
    
    // Create the kernel heap
    heap_create(&kernel_heap, (void*)heap_start_addr, (void*)heap_end_addr);

}
