#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Input functions - Read from IO ports
uint8_t inb(uint16_t port);    // Read byte from port
uint16_t inw(uint16_t port);   // Read word from port
uint32_t inl(uint16_t port);   // Read dword from port

// Output functions - Write to IO ports
void outb(uint16_t port, uint8_t value);    // Write byte to port
void outw(uint16_t port, uint16_t value);   // Write word to port
void outl(uint16_t port, uint32_t value);   // Write dword to port

// String IO functions - Read/Write multiple values
void insb(uint16_t port, void* buffer, uint32_t count);  // Read multiple bytes
void insw(uint16_t port, void* buffer, uint32_t count);  // Read multiple words
void insl(uint16_t port, void* buffer, uint32_t count);  // Read multiple dwords

void outsb(uint16_t port, const void* buffer, uint32_t count);  // Write multiple bytes
void outsw(uint16_t port, const void* buffer, uint32_t count);  // Write multiple words
void outsl(uint16_t port, const void* buffer, uint32_t count);  // Write multiple dwords

// IO wait - Small delay for slow IO devices
void io_wait(void);

#ifdef __cplusplus
}
#endif