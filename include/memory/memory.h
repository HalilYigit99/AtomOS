#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * AtomOS Memory Management Functions
 * Standard C library memory functions implementation
 */

// Basic memory operations
void* memcpy(void* dest, const void* src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
void* memchr(const void* s, int c, size_t n);

// Memory utilities
void* memccpy(void* dest, const void* src, int c, size_t n);
void* memrchr(const void* s, int c, size_t n);
void memswap(void* a, void* b, size_t n);

// Fast memory operations (optimized versions)
void* fast_memcpy(void* dest, const void* src, size_t n);
void* fast_memset(void* s, int c, size_t n);
void* fast_memmove(void* dest, const void* src, size_t n);

// Memory alignment utilities
void* memalign_copy(void* dest, const void* src, size_t n, size_t alignment);
bool is_aligned(const void* ptr, size_t alignment);
void* align_ptr(void* ptr, size_t alignment);

// Memory validation
bool is_valid_memory_range(const void* ptr, size_t size);
bool is_kernel_address(const void* ptr);
bool is_user_address(const void* ptr);

// Memory debugging utilities (for development)
void memory_dump(const void* ptr, size_t size);
void memory_pattern_fill(void* ptr, size_t size, uint32_t pattern);
bool memory_pattern_check(const void* ptr, size_t size, uint32_t pattern);

// Zero memory securely (prevents compiler optimization)
void secure_zero(void* ptr, size_t size);

// Memory copying with bounds checking
int safe_memcpy(void* dest, size_t dest_size, const void* src, size_t src_size);
int safe_memmove(void* dest, size_t dest_size, const void* src, size_t src_size);

void* kmalloc(size_t size);
void* kcalloc(size_t num, size_t size);
void* krealloc(void* ptr, size_t new_size);
void* kmalloc_aligned(size_t size, size_t alignment);
void kfree(void* ptr);

#ifdef __cplusplus
}
#endif