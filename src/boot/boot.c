#include <stddef.h>
#include <stdint.h>

#include <boot/multiboot2.h>

#include <memory/memory.h>
#include <memory/heap.h>

extern char __kernel_end; // End of kernel binary

void __kernelHeap_setup();
void __screen_fill(uint32_t color);

void __kernel_setup()
{

    // Parse multiboot2 tags first
    multiboot2_parse();

    __screen_fill(0xFFFFFFFF); // Fill screen with white color

    // Setup kernel heap
    __kernelHeap_setup();

    void *test = heap_alloc(&kernel_heap, 1024); // Example address for testing
    void* other = heap_realloc(&kernel_heap, test, 2048); // Reallocate to a larger size
    if (test == other)
    {
        // Allocation successful, do something with test
        __screen_fill(0xFF00); // Fill screen with green color as a test
    }
    else
    {
        // Allocation failed, handle error
        __screen_fill(0xFF0000); // Fill screen with red color to indicate failure
    }

}

void __kernelHeap_setup()
{
    // Check if memory map is available
    if (!mb2_mmap)
    {
        // No memory map available, cannot setup heap
        asm volatile ("cli; hlt"); // Halt the system
    }

    size_t kernelEnd = (size_t)&__kernel_end;

    heap_create(&kernel_heap, (void*)kernelEnd, (void*)(32 * 1024 * 1024)); // Create kernel heap

}

void __screen_fill(uint32_t color) {
    // Check if framebuffer info is available
    if (!mb2_framebuffer) {
        return;
    }
    
    // Get framebuffer information
    uint32_t* fb = (uint32_t*)(uintptr_t)mb2_framebuffer->framebuffer_addr;
    uint32_t width = mb2_framebuffer->framebuffer_width;
    uint32_t height = mb2_framebuffer->framebuffer_height;
    uint32_t pitch = mb2_framebuffer->framebuffer_pitch;
    uint8_t bpp = mb2_framebuffer->framebuffer_bpp;
    
    // Check framebuffer type (0 = indexed, 1 = RGB, 2 = EGA text)
    if (mb2_framebuffer->framebuffer_type != 1) {
        return; // Only support RGB type
    }
    
    // Fill the screen based on bits per pixel
    if (bpp == 32) {
        // 32-bit color mode
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                // Calculate pixel position using pitch
                uint32_t pixel_offset = (y * pitch + x * 4) / 4;
                fb[pixel_offset] = color;
            }
        }
    } else if (bpp == 24) {
        // 24-bit color mode
        uint8_t* fb8 = (uint8_t*)(uintptr_t)mb2_framebuffer->framebuffer_addr;
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                uint32_t pixel_offset = y * pitch + x * 3;
                fb8[pixel_offset] = b;     // Blue
                fb8[pixel_offset + 1] = g; // Green
                fb8[pixel_offset + 2] = r; // Red
            }
        }
    }
}