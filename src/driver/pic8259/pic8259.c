#include <driver/Driver.h>
#include <hal/irqController.h>
#include <intel86.h>
#include <io.h>
#include <print.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

extern void irq_default_handler();
extern Driver pic8259_driver;
extern IRQController pic8259_controller;
extern void intel86_isr_default();

void pic8259_driver_init(void) {
    irq_controller = &pic8259_controller;

    for (int i = 0; i < 16; i++) {
        intel86_idt_set_entry(32 + i, (uint32_t)irq_default_handler, 0x08, 0x8E);
    }

    // Initialize the PIC 
    // Reset PICs (ICW1)
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // Remap PICs (ICW2)
    outb(PIC1_DATA, 0x20);  // Master PIC vector offset (0x20)
    outb(PIC2_DATA, 0x28);  // Slave PIC vector offset (0x28)

    // Setup cascading (ICW3)
    outb(PIC1_DATA, 0x04);  // Master PIC: Slave on IRQ2
    outb(PIC2_DATA, 0x02);  // Slave PIC: Cascade identity

    // Set environment info (ICW4)
    outb(PIC1_DATA, 0x01);  // 8086/88 mode
    outb(PIC2_DATA, 0x01);  // 8086/88 mode

    // Mask all IRQs (IMR)
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);

    pic8259_driver.enabled = true;
}

void pic8259_driver_enable(void) {
    // Already enabled by default, no action needed
}

void pic8259_driver_disable(void) {
    // Cannot disable the PIC8259 driver as it is essential for IRQ handling
}

void pic8259_init(void);
void pic8259_enable(uint8_t irq);
void pic8259_disable(uint8_t irq);
void pic8259_acknowledge(uint8_t irq);
void pic8259_set_mask(uint8_t irq, bool mask);
void pic8259_set_priority(uint8_t irq, uint8_t priority);
void pic8259_set_handler(uint8_t irq, void (*handler)(void));
void pic8259_set_vector(uint8_t irq, uint8_t vector);

IRQController pic8259_controller = {
    .enable_irq = pic8259_enable,
    .disable_irq = pic8259_disable,
    .acknowledge_irq = pic8259_acknowledge,
    .set_irq_mask = pic8259_set_mask,
    .set_irq_priority = pic8259_set_priority,
    .set_irq_handler = pic8259_set_handler,
    .set_irq_vector = pic8259_set_vector
};

Driver pic8259_driver = {
    .name = "PIC8259",
    .enable = pic8259_driver_init,
    .disable = pic8259_driver_enable,
    .init = pic8259_driver_disable,
    .enabled = false,
    .type = DRIVER_TYPE_ANY,
};

void pic8259_set_mask(uint8_t irq, bool mask) {

    if (irq >= 16) {
        // Invalid IRQ number, do nothing
        return;
    }

    if (mask) {
        if (irq < 8) {
            // Mask PIC1
            uint8_t mask = inb(PIC1_DATA);
            outb(PIC1_DATA, mask | (1 << irq));
        } else {
            // Mask PIC2
            irq -= 8; // Adjust for PIC2
            uint8_t mask = inb(PIC2_DATA);
            outb(PIC2_DATA, mask | (1 << irq));
        }
    }else {
        if (irq < 8) {
            // Unmask PIC1
            uint8_t mask = inb(PIC1_DATA);
            outb(PIC1_DATA, mask & ~(1 << irq));
        } else {
            // Unmask PIC2
            irq -= 8; // Adjust for PIC2
            uint8_t mask = inb(PIC2_DATA);
            outb(PIC2_DATA, mask & ~(1 << irq));
        }
    }
}

void pic8259_acknowledge(uint8_t irq)
{
    if (irq >= 16) {
        // Invalid IRQ number, do nothing
        return;
    }

    if (irq < 8) {
        // Acknowledge PIC1
        outb(PIC1_COMMAND, 0x20); // Send EOI command to PIC1
    } else {
        // Acknowledge PIC2
        irq -= 8; // Adjust for PIC2
        outb(PIC2_COMMAND, 0x20); // Send EOI command to PIC2
    }
}

void pic8259_enable(uint8_t irq) {
    pic8259_set_mask(irq, false);
}

void pic8259_disable(uint8_t irq) {
    pic8259_set_mask(irq, true);
}

void pic8259_set_priority(uint8_t irq, uint8_t priority) {
    // Priority setting is not applicable for PIC8259, so this function does nothing
}

void pic8259_set_handler(uint8_t irq, void (*handler)(void))
{
    uint32_t handler_address = (uint32_t)handler;
    if (irq >= 16) {
        // Invalid IRQ number, do nothing
        return;
    }

    intel86_idt_set_entry(32 + irq, handler_address, 0x08, 0x8E); // Set IDT entry for the IRQ

}
void pic8259_set_vector(uint8_t irq, uint8_t vector)
{
    // Vector setting is not applicable for PIC8259, so this function does nothing
}

extern uint32_t irq2_isr_address;

void __attribute__((naked)) irq2_handler() {

    kprintf("IRQ2 handler called\n");

    irq2_isr_address = (uint32_t)irq_default_handler;

    // Slave pic e sor hangi IRQ geldi
    uint8_t irq = inb(PIC2_COMMAND) & 0x07; // Read the IRQ from PIC2
    if (irq >= 8) {
        // Send EOI to PIC2
        outb(PIC2_COMMAND, 0x20); // Send EOI command to Slave PIC
    }

    // IRQ nin ISR adresini ayarla
    uint32_t isr_addr = intel86_idt_get_isr(32 + 8 + irq);

    if (isr_addr == 0 || isr_addr == (uint32_t)intel86_isr_default) {
        irq2_isr_address = (uint32_t)irq_default_handler;
    }else {
        irq2_isr_address = isr_addr;
    }

}
