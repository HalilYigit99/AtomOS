#include <stream/OutputStream.h>
#include <stdarg.h>

#include <stream/VPrintf.h>

int uart0_open() {
    // Implementation for opening UART0
    return 0; // Return 0 on success
}

void uart0_close() {
    // Implementation for closing UART0
}

int uart0_write_char(char c) {
    // Implementation for writing a character to UART0

    return 1; // Return 1 on success
}

int uart0_write_string(const char* str) {
    // Implementation for writing a string to UART0
    while (*str) {
        uart0_write_char(*str++);
    }
    return 1;
}

int uart0_write_buffer(const void* buffer, size_t size) {
    // Implementation for writing a buffer to UART0
    const char* buf = (const char*)buffer;
    for (size_t i = 0; i < size; i++) {
        uart0_write_char(buf[i]);
    }
    return size; // Return number of bytes written
}

void uart0_print(const char* str) {
    uart0_write_string(str);
}

void uart0_printf(const char* format, ...) {
    // Implementation for formatted printing to UART0
    va_list args;
    va_start(args, format);
    
    // Assuming a simple implementation that just prints the string
    // A full implementation would handle format specifiers
    vprintf(uart0_write_char, format, args);
    
    va_end(args);
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

