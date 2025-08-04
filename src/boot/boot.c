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
void __screen_fill(uint32_t color);

void terminal_init();

void timer_handle()
{
    // Timer interrupt handler
}

void timer_isr();

void __kernel_setup()
{
    // Initialize output stream
    currentOutputStream->Open();

    // Parse multiboot2 tags first
    multiboot2_parse();

    // Setup kernel heap
    __kernelHeap_setup();

    // Initialize graphics subsystem
    gfx_init();

    // Initialize PCI subsystem
    pci_init();

    // Initialize terminal
    terminal_init();

    // Setup timer interrupt (IRQ0 -> INT 32)
    intel86_idt_set_entry(32, (uint32_t)timer_isr, 0x08, 0x8E);
    pic_unmask(0);

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
