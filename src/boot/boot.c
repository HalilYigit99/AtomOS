#include <stddef.h>
#include <stdint.h>
#include <boot/multiboot2.h>
#include <memory/memory.h>
#include <memory/heap.h>
#include <stream/OutputStream.h>
#include <pci/pci.h>
#include <convert.h>
#include <intel86.h>
#include <driver/Driver.h>
#include <keyboard/Keyboard.h>
#include <graphics/gfx.h>

extern char __kernel_end; // End of kernel binary

void __kernelHeap_setup();
void __screen_fill(uint32_t color);
void gfx_init();

uint32_t screenColor = 0xFFFFFFFF; // Default screen color (white)

void timer_handle()
{
}

void timer_isr();

void __kernel_setup()
{

    currentOutputStream->Open();

    // Parse multiboot2 tags first
    multiboot2_parse();

    currentOutputStream->printf("Kernel setup started...\n");

    // Setup kernel heap
    __kernelHeap_setup();

    gfx_init(); // Initialize graphics subsystem

    currentOutputStream->printf("Kernel heap setup completed.\n");

    pci_init(); // Initialize PCI subsystem

    intel86_idt_set_entry(32, (uint32_t)timer_isr, 0x08, 0x8E); // Set timer ISR in IDT

    pic_unmask(0);

    sys_driver_register(&ps2kbd_driver); // Register PS/2 keyboard driver

    __screen_fill(0);

    gfx_draw_line(screen_buffer, (gfx_line){
        .start = {0, 0},
        .end = {100, 100},
        .color = {0xFF, 0xFF, 0xFF, 0xFF}, // Red color
        .thickness = 1
    });

    gfx_draw_pixel(screen_buffer, (gfx_point){0, 0}, (gfx_color){0xFF, 0x00, 0xFF, 0xFF}); // Draw a red pixel at (0, 0)

    gfx_draw_char(screen_buffer, (gfx_point){10, 10}, 'A', (gfx_color){0xFF, 0xFF, 0x00, 0xFF}); // Draw a red 'A' at (10, 10)

}

void __kernelHeap_setup()
{
    // Check if memory map is available
    if (!mb2_mmap)
    {
        // No memory map available, cannot setup heap
        asm volatile("cli; hlt"); // Halt the system
    }

    size_t kernelEnd = (size_t)&__kernel_end;

    heap_create(&kernel_heap, (void *)kernelEnd, (void *)(32 * 1024 * 1024)); // Create kernel heap
}

void __screen_fill(uint32_t color)
{
    // Check if framebuffer info is available
    if (!mb2_framebuffer)
    {
        return;
    }

    // Get framebuffer information
    uint32_t *fb = (uint32_t *)(uintptr_t)mb2_framebuffer->framebuffer_addr;
    uint32_t width = mb2_framebuffer->framebuffer_width;
    uint32_t height = mb2_framebuffer->framebuffer_height;
    uint32_t pitch = mb2_framebuffer->framebuffer_pitch;
    uint8_t bpp = mb2_framebuffer->framebuffer_bpp;

    // Check framebuffer type (0 = indexed, 1 = RGB, 2 = EGA text)
    if (mb2_framebuffer->framebuffer_type != 1)
    {
        return; // Only support RGB type
    }

    // Fill the screen based on bits per pixel
    if (bpp == 32)
    {
        // 32-bit color mode
        for (uint32_t y = 0; y < height; y++)
        {
            for (uint32_t x = 0; x < width; x++)
            {
                // Calculate pixel position using pitch
                uint32_t pixel_offset = (y * pitch + x * 4) / 4;
                fb[pixel_offset] = color;
            }
        }
    }
    else if (bpp == 24)
    {
        // 24-bit color mode
        uint8_t *fb8 = (uint8_t *)(uintptr_t)mb2_framebuffer->framebuffer_addr;
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;

        for (uint32_t y = 0; y < height; y++)
        {
            for (uint32_t x = 0; x < width; x++)
            {
                uint32_t pixel_offset = y * pitch + x * 3;
                fb8[pixel_offset] = b;     // Blue
                fb8[pixel_offset + 1] = g; // Green
                fb8[pixel_offset + 2] = r; // Red
            }
        }
    }
}