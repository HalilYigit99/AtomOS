#include <stddef.h>
#include <stdint.h>

#define IDT_SIZE 256

typedef struct {
    uint16_t offset_low;   // Handler address low 16 bits
    uint16_t selector;     // Code segment selector
    uint8_t  zero;         // Always 0
    uint8_t  type_attr;    // Type and attributes
    uint16_t offset_high;  // Handler address high 16 bits
} __attribute__((packed)) idt_entry_t;

extern idt_entry_t intel86_idt[];
extern uint16_t intel86_idtr[];
extern void intel86_isr_default();

void intel86_idt_set_entry(size_t index, uint32_t base, uint16_t selector, uint8_t type_attr) {
    if (index >= IDT_SIZE) {
        return; // Index out of bounds
    }

    intel86_idt[index].offset_low = base & 0xFFFF;
    intel86_idt[index].offset_high = (base >> 16) & 0xFFFF;
    intel86_idt[index].selector = selector;
    intel86_idt[index].zero = 0;
    intel86_idt[index].type_attr = type_attr;
}

void intel86_idt_init(void) {
    // Initialize the IDT entries
    for (size_t i = 0; i < IDT_SIZE; i++) {
        intel86_idt_set_entry(i, (uint32_t)(uintptr_t)intel86_isr_default, 0x08, 0x8E);
    }

    // Load the IDT using IDTR
    asm volatile("lidt %0" : : "m"(intel86_idtr));
}
