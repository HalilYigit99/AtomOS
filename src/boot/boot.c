#include <stddef.h>
#include <stdint.h>
#include <boot/multiboot2.h>
#include <memory/memory.h>
#include <memory/heap.h>
#include <stream/OutputStream.h>
#include <pci/pci.h>
#include <intel86.h>
#include <driver/Driver.h>
#include <keyboard/Keyboard.h>
#include <graphics/gfx.h>

extern char __kernel_end; // End of kernel binary

void __kernelHeap_setup();
void gfx_init();

void __kernel_setup()
{
    // Initialize output stream
    currentOutputStream->Open();

    currentOutputStream->printf("Kernel setup started...\n");

    // Parse multiboot2 tags first
    multiboot2_parse();

    currentOutputStream->printf("Multiboot2 tags parsed successfully.\n");

    // Setup kernel heap
    __kernelHeap_setup();

    currentOutputStream->printf("Kernel heap setup completed.\n");

    // Setup gfx subsystem
    gfx_init();

    currentOutputStream->printf("Graphics subsystem initialized.\n");

    // Initialize PCI subsystem
    pci_init();

    // Register PS/2 keyboard driver
    sys_driver_register(&ps2kbd_driver);

}

void __kernelHeap_setup()
{
    // Check if memory map is available
    if (!mb2_mmap) {
        // No memory map available, cannot setup heap
        asm volatile("cli; hlt"); // Halt the system
    }

    size_t kernelEnd = (size_t)&__kernel_end;
    
    // Create kernel heap (32MB heap space)
    heap_create(&kernel_heap, (void*)kernelEnd, (void*)(64 * 1024 * 1024));
}
