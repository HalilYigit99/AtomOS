#include <memory/memory.h>
#include <stdbool.h>

// Memory alignment macros
#define WORD_SIZE sizeof(void*)
#define ALIGN_MASK (WORD_SIZE - 1)
#define IS_WORD_ALIGNED(ptr) (((uintptr_t)(ptr) & ALIGN_MASK) == 0)

// Memory boundaries removed - allow operations on all memory areas

// Basic memory operations
void* memcpy(void* dest, const void* src, size_t n) {
    if (!dest || !src || n == 0) {
        return dest;
    }

    char* d = (char*)dest;
    const char* s = (const char*)src;

    // Fast path for word-aligned addresses
    if (IS_WORD_ALIGNED(d) && IS_WORD_ALIGNED(s) && n >= WORD_SIZE) {
        size_t* wd = (size_t*)d;
        const size_t* ws = (const size_t*)s;
        size_t words = n / WORD_SIZE;

        while (words--) {
            *wd++ = *ws++;
        }

        d = (char*)wd;
        s = (const char*)ws;
        n %= WORD_SIZE;
    }

    // Copy remaining bytes
    while (n--) {
        *d++ = *s++;
    }

    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    if (!dest || !src || n == 0) {
        return dest;
    }

    char* d = (char*)dest;
    const char* s = (const char*)src;

    // Check for overlap
    if (d < s || d >= s + n) {
        // No overlap, use memcpy
        return memcpy(dest, src, n);
    }

    // Overlap exists, copy backwards
    d += n - 1;
    s += n - 1;

    while (n--) {
        *d-- = *s--;
    }

    return dest;
}

void* memset(void* s, int c, size_t n) {
    if (!s || n == 0) {
        return s;
    }

    char* p = (char*)s;
    unsigned char uc = (unsigned char)c;

    // Fast path for word-aligned addresses and large blocks
    if (IS_WORD_ALIGNED(p) && n >= WORD_SIZE) {
        size_t* wp = (size_t*)p;
        size_t word_pattern = 0;

        // Create word-sized pattern
        for (size_t i = 0; i < WORD_SIZE; i++) {
            word_pattern = (word_pattern << 8) | uc;
        }

        size_t words = n / WORD_SIZE;
        while (words--) {
            *wp++ = word_pattern;
        }

        p = (char*)wp;
        n %= WORD_SIZE;
    }

    // Set remaining bytes
    while (n--) {
        *p++ = uc;
    }

    return s;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    if (!s1 || !s2 || n == 0) {
        return 0;
    }

    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;

    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }

    return 0;
}

void* memchr(const void* s, int c, size_t n) {
    if (!s || n == 0) {
        return NULL;
    }

    const unsigned char* p = (const unsigned char*)s;
    unsigned char uc = (unsigned char)c;

    while (n--) {
        if (*p == uc) {
            return (void*)p;
        }
        p++;
    }

    return NULL;
}

// Memory utilities
void* memccpy(void* dest, const void* src, int c, size_t n) {
    if (!dest || !src || n == 0) {
        return NULL;
    }

    char* d = (char*)dest;
    const char* s = (const char*)src;
    unsigned char uc = (unsigned char)c;

    while (n--) {
        *d = *s;
        if (*s == uc) {
            return d + 1;
        }
        d++;
        s++;
    }

    return NULL;
}

void* memrchr(const void* s, int c, size_t n) {
    if (!s || n == 0) {
        return NULL;
    }

    const unsigned char* p = (const unsigned char*)s + n - 1;
    unsigned char uc = (unsigned char)c;

    while (n--) {
        if (*p == uc) {
            return (void*)p;
        }
        p--;
    }

    return NULL;
}

void memswap(void* a, void* b, size_t n) {
    if (!a || !b || n == 0 || a == b) {
        return;
    }

    char* pa = (char*)a;
    char* pb = (char*)b;

    while (n--) {
        char temp = *pa;
        *pa = *pb;
        *pb = temp;
        pa++;
        pb++;
    }
}

// Fast memory operations
void* fast_memcpy(void* dest, const void* src, size_t n) {
    if (!dest || !src || n == 0) {
        return dest;
    }

    // Use 64-bit copies when possible
    if (IS_WORD_ALIGNED(dest) && IS_WORD_ALIGNED(src) && n >= 8) {
        uint64_t* d64 = (uint64_t*)dest;
        const uint64_t* s64 = (const uint64_t*)src;

        while (n >= 8) {
            *d64++ = *s64++;
            n -= 8;
        }

        if (n >= 4) {
            *(uint32_t*)d64 = *(const uint32_t*)s64;
            d64 = (uint64_t*)((char*)d64 + 4);
            s64 = (const uint64_t*)((const char*)s64 + 4);
            n -= 4;
        }

        char* d = (char*)d64;
        const char* s = (const char*)s64;

        while (n--) {
            *d++ = *s++;
        }
    } else {
        return memcpy(dest, src, n);
    }

    return dest;
}

void* fast_memset(void* s, int c, size_t n) {
    if (!s || n == 0) {
        return s;
    }

    if (IS_WORD_ALIGNED(s) && n >= 8) {
        uint64_t* p64 = (uint64_t*)s;
        uint64_t pattern = (unsigned char)c;
        pattern |= pattern << 8;
        pattern |= pattern << 16;
        pattern |= pattern << 32;

        while (n >= 8) {
            *p64++ = pattern;
            n -= 8;
        }

        char* p = (char*)p64;
        while (n--) {
            *p++ = (unsigned char)c;
        }
    } else {
        return memset(s, c, n);
    }

    return s;
}

void* fast_memmove(void* dest, const void* src, size_t n) {
    // For non-overlapping regions, use fast_memcpy
    if ((char*)dest + n <= (const char*)src || (const char*)src + n <= (char*)dest) {
        return fast_memcpy(dest, src, n);
    }
    return memmove(dest, src, n);
}

// Memory alignment utilities
void* memalign_copy(void* dest, const void* src, size_t n, size_t alignment) {
    if (!is_aligned(dest, alignment) || !is_aligned(src, alignment)) {
        return memcpy(dest, src, n);
    }
    return fast_memcpy(dest, src, n);
}

bool is_aligned(const void* ptr, size_t alignment) {
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return false; // alignment must be power of 2
    }
    return ((uintptr_t)ptr & (alignment - 1)) == 0;
}

void* align_ptr(void* ptr, size_t alignment) {
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return ptr; // Invalid alignment
    }
    uintptr_t addr = (uintptr_t)ptr;
    return (void*)((addr + alignment - 1) & ~(alignment - 1));
}

// Memory validation (placeholder implementations)
bool is_valid_memory_range(const void* ptr, size_t size) {
    if (!ptr || size == 0) {
        return false;
    }
    
    uintptr_t start = (uintptr_t)ptr;
    uintptr_t end = start + size - 1;
    
    // Check for overflow
    if (end < start) {
        return false;
    }
    
    // Add your specific memory range validation here
    return true;
}

bool is_kernel_address(const void* ptr) {
    // Boundaries removed - allow all memory operations
    (void)ptr; // Suppress unused parameter warning
    return true;
}

bool is_user_address(const void* ptr) {
    // Boundaries removed - allow all memory operations
    (void)ptr; // Suppress unused parameter warning
    return true;
}

void memory_pattern_fill(void* ptr, size_t size, uint32_t pattern) {
    if (!ptr || size == 0) {
        return;
    }

    uint32_t* p32 = (uint32_t*)ptr;
    size_t words = size / sizeof(uint32_t);

    while (words--) {
        *p32++ = pattern;
    }

    // Fill remaining bytes
    unsigned char* p8 = (unsigned char*)p32;
    size_t remaining = size % sizeof(uint32_t);
    unsigned char* pattern_bytes = (unsigned char*)&pattern;

    for (size_t i = 0; i < remaining; i++) {
        p8[i] = pattern_bytes[i];
    }
}

bool memory_pattern_check(const void* ptr, size_t size, uint32_t pattern) {
    if (!ptr || size == 0) {
        return false;
    }

    const uint32_t* p32 = (const uint32_t*)ptr;
    size_t words = size / sizeof(uint32_t);

    while (words--) {
        if (*p32++ != pattern) {
            return false;
        }
    }

    // Check remaining bytes
    const unsigned char* p8 = (const unsigned char*)p32;
    size_t remaining = size % sizeof(uint32_t);
    const unsigned char* pattern_bytes = (const unsigned char*)&pattern;

    for (size_t i = 0; i < remaining; i++) {
        if (p8[i] != pattern_bytes[i]) {
            return false;
        }
    }

    return true;
}

// Secure zero (prevents compiler optimization)
void secure_zero(void* ptr, size_t size) {
    if (!ptr || size == 0) {
        return;
    }

    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (size--) {
        *p++ = 0;
    }
}

// Safe memory operations with bounds checking
int safe_memcpy(void* dest, size_t dest_size, const void* src, size_t src_size) {
    if (!dest || !src || dest_size == 0 || src_size == 0) {
        return -1;
    }

    size_t copy_size = (src_size < dest_size) ? src_size : dest_size;
    memcpy(dest, src, copy_size);
    
    return (src_size <= dest_size) ? 0 : 1; // 1 indicates truncation
}

int safe_memmove(void* dest, size_t dest_size, const void* src, size_t src_size) {
    if (!dest || !src || dest_size == 0 || src_size == 0) {
        return -1;
    }

    size_t copy_size = (src_size < dest_size) ? src_size : dest_size;
    memmove(dest, src, copy_size);
    
    return (src_size <= dest_size) ? 0 : 1; // 1 indicates truncation
}