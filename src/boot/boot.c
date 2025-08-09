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

extern Driver ps2kbd_driver; // PS/2 keyboard driver
extern Driver ps2mouse_driver; // PS/2 mouse driver
extern Driver pic8259_driver; // PIC8259 driver


void __kernelHeap_setup();
void gfx_init();
void acpi_init();
void intel86_setup();

void intel86_pit_init();

void __kernel_setup()
{

    intel86_setup();

    // Initialize output stream
    currentOutputStream->Open();

    currentOutputStream->printf("Kernel setup started...\n");

    // Parse multiboot2 tags first
    multiboot2_parse();

    currentOutputStream->printf("Multiboot2 tags parsed successfully.\n");

    // Setup kernel heap
    __kernelHeap_setup();

    currentOutputStream->printf("Kernel heap setup completed.\n");

    // Initialize ACPI subsystem
    // acpi_init();

    currentOutputStream->printf("ACPI subsystem initialized.\n");

    // Setup gfx subsystem
    gfx_init();

    currentOutputStream->printf("Graphics subsystem initialized.\n");

    // Initialize PCI subsystem
    pci_init();

    // Initialize PS/2 keyboard driver
    sys_driver_register(&ps2kbd_driver);

    // // Register PS/2 keyboard driver
    // sys_driver_register(&ps2kbd_driver);

    // // Register PS/2 mouse driver
    // sys_driver_register(&ps2mouse_driver);

    intel86_pit_init();

}

void __kernelHeap_setup()
{

    size_t kernelEnd = (size_t)&__kernel_end;
    
    // Create kernel heap (32MB heap space)
    heap_create(&kernel_heap, (void*)kernelEnd, (void*)(64 * 1024 * 1024));
}
