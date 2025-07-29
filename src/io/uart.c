#include <stream/OutputStream.h>
#include <stdarg.h>
#include <stream/VPrintf.h>
#include <io.h>

// UART0 (COM1) Base Port Address
#define UART0_BASE 0x3F8

// UART Register Offsets
#define UART_DATA_REG           0   // Data Register (R/W)
#define UART_INTERRUPT_REG      1   // Interrupt Enable Register
#define UART_DIVISOR_LOW        0   // Divisor Latch Low (when DLAB=1)
#define UART_DIVISOR_HIGH       1   // Divisor Latch High (when DLAB=1)
#define UART_INT_ID_REG         2   // Interrupt ID Register (Read)
#define UART_FIFO_CTRL_REG      2   // FIFO Control Register (Write)
#define UART_LINE_CTRL_REG      3   // Line Control Register
#define UART_MODEM_CTRL_REG     4   // Modem Control Register
#define UART_LINE_STATUS_REG    5   // Line Status Register
#define UART_MODEM_STATUS_REG   6   // Modem Status Register
#define UART_SCRATCH_REG        7   // Scratch Register

// Line Status Register Bits
#define UART_LSR_DATA_READY     0x01    // Data ready
#define UART_LSR_OVERRUN_ERR    0x02    // Overrun error
#define UART_LSR_PARITY_ERR     0x04    // Parity error
#define UART_LSR_FRAME_ERR      0x08    // Framing error
#define UART_LSR_BREAK_INT      0x10    // Break interrupt
#define UART_LSR_THR_EMPTY      0x20    // Transmitter holding register empty
#define UART_LSR_TX_EMPTY       0x40    // Transmitter empty
#define UART_LSR_FIFO_ERR       0x80    // FIFO error

// Line Control Register Bits
#define UART_LCR_DLAB           0x80    // Divisor Latch Access Bit

// Modem Control Register Bits
#define UART_MCR_DTR            0x01    // Data Terminal Ready
#define UART_MCR_RTS            0x02    // Request To Send
#define UART_MCR_OUT1           0x04    // Output 1
#define UART_MCR_OUT2           0x08    // Output 2 (Enable IRQ)
#define UART_MCR_LOOP           0x10    // Loopback mode

// FIFO Control Register Bits
#define UART_FCR_ENABLE_FIFO    0x01    // Enable FIFOs
#define UART_FCR_CLEAR_RX       0x02    // Clear receive FIFO
#define UART_FCR_CLEAR_TX       0x04    // Clear transmit FIFO
#define UART_FCR_DMA_SELECT     0x08    // DMA mode select
#define UART_FCR_TRIGGER_14     0xC0    // Trigger level 14 bytes

static int uart0_initialized = 0;

int uart0_open() {
    if (uart0_initialized) {
        return 0; // Already initialized
    }

    // Disable interrupts
    outb(UART0_BASE + UART_INTERRUPT_REG, 0x00);

    // Enable DLAB (Divisor Latch Access Bit)
    outb(UART0_BASE + UART_LINE_CTRL_REG, UART_LCR_DLAB);

    // Set baud rate divisor for 115200 bps
    // Divisor = 115200 / desired_baud_rate
    // For 115200 bps: divisor = 1
    outb(UART0_BASE + UART_DIVISOR_LOW, 0x01);   // Low byte
    outb(UART0_BASE + UART_DIVISOR_HIGH, 0x00);  // High byte

    // 8 bits, no parity, 1 stop bit (8N1)
    outb(UART0_BASE + UART_LINE_CTRL_REG, 0x03);

    // Enable FIFO, clear them, with 14-byte threshold
    outb(UART0_BASE + UART_FIFO_CTRL_REG, 
         UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RX | 
         UART_FCR_CLEAR_TX | UART_FCR_TRIGGER_14);

    // Enable IRQs, set RTS/DTR
    outb(UART0_BASE + UART_MODEM_CTRL_REG, 
         UART_MCR_DTR | UART_MCR_RTS | UART_MCR_OUT2);

    // Enable interrupts (optional - for now we'll use polling)
    // outb(UART0_BASE + UART_INTERRUPT_REG, 0x01);

    uart0_initialized = 1;
    return 0; // Success
}

void uart0_close() {
    if (!uart0_initialized) {
        return;
    }

    // Disable interrupts
    outb(UART0_BASE + UART_INTERRUPT_REG, 0x00);

    // Disable FIFO
    outb(UART0_BASE + UART_FIFO_CTRL_REG, 0x00);

    // Clear DTR and RTS
    outb(UART0_BASE + UART_MODEM_CTRL_REG, 0x00);

    uart0_initialized = 0;
}

// Wait until transmitter is ready
static int uart0_is_transmit_empty() {
    return inb(UART0_BASE + UART_LINE_STATUS_REG) & UART_LSR_THR_EMPTY;
}

// Check if data is available to read
static int __attribute__((unused)) uart0_is_data_ready() {
    return inb(UART0_BASE + UART_LINE_STATUS_REG) & UART_LSR_DATA_READY;
}

int uart0_write_char(char c) {
    if (!uart0_initialized) {
        return 0; // Not initialized
    }

    // Wait until transmitter is ready
    while (!uart0_is_transmit_empty()) {
        // Could add timeout here to prevent infinite loop
        io_wait();
    }

    // Send the character
    outb(UART0_BASE + UART_DATA_REG, c);

    // Handle newline - send CR+LF
    if (c == '\n') {
        while (!uart0_is_transmit_empty()) {
            io_wait();
        }
        outb(UART0_BASE + UART_DATA_REG, '\r');
    }

    return 1; // Success
}

int uart0_write_string(const char* str) {
    if (!uart0_initialized || !str) {
        return 0;
    }

    int count = 0;
    while (*str) {
        if (!uart0_write_char(*str++)) {
            return count; // Return number of chars written before error
        }
        count++;
    }
    return count;
}

int uart0_write_buffer(const void* buffer, size_t size) {
    if (!uart0_initialized || !buffer) {
        return 0;
    }

    const char* buf = (const char*)buffer;
    for (size_t i = 0; i < size; i++) {
        if (!uart0_write_char(buf[i])) {
            return i; // Return number of bytes written before error
        }
    }
    return size;
}

int uart0_print(const char* str) {
    return uart0_write_string(str) > 0 ? 1 : 0;
}

// Wrapper function for vprintf compatibility
static void uart0_putchar_wrapper(char c) {
    uart0_write_char(c);
}

int uart0_printf(const char* format, ...) {
    if (!uart0_initialized) {
        return 0;
    }

    va_list args;
    va_start(args, format);
    
    // Use our vprintf implementation with wrapper function
    vprintf(uart0_putchar_wrapper, format, args);

    va_end(args);
    return 1;
}

// Check if UART0 exists on this system
int uart0_exists() {
    // Save original scratch register value
    uint8_t original = inb(UART0_BASE + UART_SCRATCH_REG);
    
    // Write test pattern
    outb(UART0_BASE + UART_SCRATCH_REG, 0xAA);
    io_wait();
    
    // Check if we can read it back
    if (inb(UART0_BASE + UART_SCRATCH_REG) != 0xAA) {
        return 0; // UART not present
    }
    
    // Write another test pattern
    outb(UART0_BASE + UART_SCRATCH_REG, 0x55);
    io_wait();
    
    if (inb(UART0_BASE + UART_SCRATCH_REG) != 0x55) {
        return 0; // UART not present
    }
    
    // Restore original value
    outb(UART0_BASE + UART_SCRATCH_REG, original);
    
    return 1; // UART exists
}

OutputStream __uart0_output_stream = {
    .Open = uart0_open,
    .Close = uart0_close,
    .writeChar = uart0_write_char,
    .writeString = uart0_write_string,
    .writeBuffer = uart0_write_buffer,
    .print = uart0_print,
    .printf = uart0_printf
};
