#include <intel86.h>
#include <hal/irqController.h>
#include <stream/OutputStream.h>
#include <list.h>

void pit_isr_handler();
void executePeriodicTasks(uint32_t currentTime);

#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40

// 1193182 Hz / 1000 Hz = 1193
#define PIT_FREQUENCY_DIVISOR 1193

uint32_t uptime_ms = 0;

void intel86_pit_init() {

    // Mask PIT interrupts
    irq_controller->disable_irq(0);
    
    // Komut portuna: kanal 0, erişim low+high, mod 2 (rate generator), binary mode
    outb(PIT_COMMAND_PORT, 0x36);  // 00 11 011 0

    // Divisor (low byte sonra high byte olarak gönder)
    uint16_t divisor = PIT_FREQUENCY_DIVISOR;
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF);       // Low byte
    outb(PIT_CHANNEL0_PORT, (divisor >> 8) & 0xFF); // High byte

    currentOutputStream->printf("Initializing PIT with divisor %u for 1000Hz...\n", divisor);

    // Set up IDT entry for PIT (IRQ0 -> Interrupt 0x20)
    irq_controller->set_irq_handler(0, pit_isr_handler);

    // Unmask IRQ0 in the PIC to enable PIT interrupts
    irq_controller->enable_irq(0);
    
    currentOutputStream->printf("PIT initialized successfully!\n");
}

void pit_interrupt_handler() {

    // Increment uptime counter
    uptime_ms++;

    // Execute periodic tasks
    executePeriodicTasks(uptime_ms);
}