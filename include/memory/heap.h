#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This file is a placeholder for the memory heap management header.
 * It should be implemented with the necessary functions and macros for heap memory management.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define HEAP_MAGIC 0xDEADBEEF

typedef struct __heap_block_t {
    size_t size;          // Size of the block
    struct __heap_block_t* next; // Pointer to the next block in the free list
    bool free;           // Is this block free?
    uint32_t magic;      // Magic number for validation
} heap_block_t;

typedef struct {
    void* start;       // Start address of the heap
    void* end;         // End address of the heap
    size_t size;       // Total size of the heap
    size_t free;       // Free size in the heap
} heap_t;

extern heap_t kernel_heap; // Global kernel heap

void heap_create(heap_t* heap, void* start, void* end);
void* heap_alloc(heap_t* heap, size_t size);
void heap_free(heap_t* heap, void* ptr);
void heap_destroy(heap_t* heap);

void* heap_realloc(heap_t* heap, void* ptr, size_t new_size);
void* heap_calloc(heap_t* heap, size_t num, size_t size);

void* heap_alloc_aligned(heap_t* heap, size_t size, size_t alignment);

#ifdef __cplusplus
}
#endif
