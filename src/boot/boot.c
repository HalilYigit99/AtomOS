#include <stddef.h>
#include <stdint.h>
#include <boot/multiboot2.h>
#include <memory/memory.h>
#include <memory/heap.h>
#include <stream/OutputStream.h>
#include <pci/pci.h>

extern char __kernel_end; // End of kernel binary

void __kernelHeap_setup();
void __screen_fill(uint32_t color);

void __kernel_setup()
{

    currentOutputStream->Open();

    // Parse multiboot2 tags first
    multiboot2_parse();

    currentOutputStream->printf("Kernel setup started...\n");

    __screen_fill(0xFFFFFFFF); // Fill screen with white color

    // Setup kernel heap
    __kernelHeap_setup();

    currentOutputStream->printf("Kernel heap setup completed.\n");

    pci_init(); // Initialize PCI subsystem

    currentOutputStream->printf("\n======= PCI Devices =======\n");
    pci_print_all_devices();
    currentOutputStream->printf("\n======= PCI Devices =======\n");

    currentOutputStream->printf("========== gcc.asm test ==========\n");
    uint64_t a = 1000000;
    uint64_t b = 2000000;
    uint64_t c = 3000000;
    uint64_t result = a + b + c;
    currentOutputStream->printf("gcc.asm test: %llu + %llu + %llu = %llu\n", a, b, c, result);

    // Multiply test
    uint64_t x = 123;
    uint64_t y = 123;
    uint64_t product = x * y;
    currentOutputStream->printf("gcc.asm multiply test: %llu * %llu = %llu\n", x, y, product);

    // Division test

    uint64_t dividend = 1000000000;
    uint64_t divisor = 123456;
    uint64_t quotient = dividend / divisor;
    uint64_t remainder = dividend % divisor;
    currentOutputStream->printf("gcc.asm division test: %llu / %llu = %llu, remainder = %llu\n", 
                                dividend, divisor, quotient, remainder);

    // Shift test
    uint64_t shift_value = 1;
    uint64_t left_shift = shift_value << 3; // Shift left by 3
    uint64_t right_shift = shift_value >> 2; // Shift right by 2
    currentOutputStream->printf("gcc.asm shift test: %llu << 3 = %llu, %llu >> 2 = %llu\n", 
                                shift_value, left_shift, right_shift, right_shift);

    // Bitwise AND test
    uint64_t and_value1 = 0xF0F0F0F0;
    uint64_t and_value2 = 0x0F0F0F0F;
    uint64_t and_result = and_value1 & and_value2;
    currentOutputStream->printf("gcc.asm AND test: 0x%llX & 0x%llX = 0x%llX\n", 
                                and_value1, and_value2, and_result);

    uint64_t aa = 50000;
    uint64_t bb = 10000;
    if (aa / bb == 5) {
        currentOutputStream->printf("gcc.asm division test passed: %llu / %llu = 5\n", aa, bb);
    } else {
        currentOutputStream->printf("gcc.asm division test failed: %llu / %llu != 5\n", aa, bb);
    }

    currentOutputStream->printf("========== gcc.asm test completed ==========\n");

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