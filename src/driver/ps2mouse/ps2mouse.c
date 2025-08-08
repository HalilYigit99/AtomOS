#include <intel86.h>
#include <driver/Driver.h>
#include <print.h>
#include <driver/ps2mouse/ps2mouse.h>

static bool enabled = false;

extern int cursor_X; // Current cursor X position
extern int cursor_Y; // Current cursor Y position

extern void ps2mouse_isr(void);

// Helper functions for PS/2 controller communication
void ps2mouse_wait_write(void) {
    // Wait until input buffer is empty (bit 1 clear)
    int timeout = 100000;
    while ((inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_FULL) && timeout--) {
        // Small delay
        asm volatile("pause");
    }
    if (timeout == 0) {
        kprintf("PS/2 Mouse: Write timeout!\n");
    }
}

void ps2mouse_wait_read(void) {
    // Wait until output buffer is full (bit 0 set)
    int timeout = 100000;
    while (!(inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) && timeout--) {
        // Small delay
        asm volatile("pause");
    }
    if (timeout == 0) {
        kprintf("PS/2 Mouse: Read timeout!\n");
    }
}

uint8_t ps2mouse_read_data(void) {
    ps2mouse_wait_read();
    return inb(PS2_DATA_PORT);
}

void ps2mouse_write_command(uint8_t cmd) {
    ps2mouse_wait_write();
    outb(PS2_COMMAND_PORT, cmd);
}

void ps2mouse_write_data(uint8_t data) {
    ps2mouse_wait_write();
    outb(PS2_DATA_PORT, data);
}

// Send command to mouse (not controller)
void ps2mouse_send_command(uint8_t cmd) {
    ps2mouse_write_command(PS2_CMD_WRITE_TO_AUX);
    ps2mouse_write_data(cmd);
    
    // Wait for ACK
    uint8_t response = ps2mouse_read_data();
    if (response != PS2_MOUSE_ACK) {
        kprintf("PS/2 Mouse: Command 0x%02X failed, got 0x%02X\n", cmd, response);
    }
}

void ps2mouse_init(void) {
    kprintf("PS/2 mouse hardware initializing...\n");
    
    // Step 1: Enable auxiliary device (mouse port)
    ps2mouse_write_command(PS2_CMD_ENABLE_AUX);
    
    // Step 2: Get current controller configuration
    ps2mouse_write_command(PS2_CMD_READ_CONFIG);
    uint8_t config = ps2mouse_read_data();
    
    // Step 3: Enable mouse interrupt (IRQ12) and mouse clock
    config |= 0x02;   // Enable interrupt for auxiliary port (bit 1)
    config &= ~0x20;  // Enable auxiliary clock (clear bit 5)
    
    // Step 4: Write back configuration
    ps2mouse_write_command(PS2_CMD_WRITE_CONFIG);
    ps2mouse_write_data(config);
    
    // Step 5: Reset mouse
    kprintf("PS/2 Mouse: Resetting mouse...\n");
    ps2mouse_write_command(PS2_CMD_WRITE_TO_AUX);
    ps2mouse_write_data(PS2_MOUSE_CMD_RESET);
    
    // Wait for ACK
    uint8_t ack = ps2mouse_read_data();
    if (ack != PS2_MOUSE_ACK) {
        kprintf("PS/2 Mouse: Reset ACK failed: 0x%02X\n", ack);
    }
    
    // Wait for self-test result (0xAA for success)
    uint8_t test_result = ps2mouse_read_data();
    if (test_result != 0xAA) {
        kprintf("PS/2 Mouse: Self-test failed: 0x%02X\n", test_result);
    }
    
    // Wait for device ID (usually 0x00 for standard mouse)
    uint8_t device_id = ps2mouse_read_data();
    kprintf("PS/2 Mouse: Device ID: 0x%02X\n", device_id);
    
    // Step 6: Set defaults
    ps2mouse_send_command(PS2_MOUSE_CMD_SET_DEFAULTS);
    
    // Step 7: Disable data reporting
    ps2mouse_send_command(PS2_MOUSE_CMD_DISABLE_REPORTING);
    
    // Step 8: Setup interrupt handler BEFORE unmasking
    intel86_idt_set_entry(32 + 12, (uint32_t)ps2mouse_isr, 0x08, 0x8E);
    
    // Step 9: Clear any pending data
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) {
        inb(PS2_DATA_PORT);
    }
    
    kprintf("PS/2 mouse hardware initialized!\n");
}

void ps2mouse_enable(void) {
    if (!enabled) {
        // Send enable command to mouse
        ps2mouse_send_command(PS2_MOUSE_CMD_ENABLE_REPORTING);
        pic_unmask(12);
        enabled = true;
        kprintf("PS/2 Mouse: Enabled\n");
    }
}

void ps2mouse_disable(void) {
    if (enabled) {
        // Send disable command to mouse
        ps2mouse_send_command(PS2_MOUSE_CMD_DISABLE_REPORTING);
        pic_mask(12);
        enabled = false;
        kprintf("PS/2 Mouse: Disabled\n");
    }
}

bool ps2mouse_enabled(void) {
    return enabled;
}

static uint8_t packet_buffer[3];
static size_t packet_index = 0;

void ps2mouse_isr_handler(void) {

    kprintf("How do we get here?\n");

    // Read status to check if data is from mouse
    uint8_t status = inb(PS2_STATUS_PORT);
    
    // Check if data is available and it's from mouse (AUX)
    if (!(status & PS2_STATUS_OUTPUT_FULL)) {
        return;  // No data available
    }
    
    // Check if data is from mouse (bit 5 should be set for AUX data)
    if (!(status & PS2_STATUS_AUX_DATA)) {
        // Data is from keyboard, not mouse - read and discard
        inb(PS2_DATA_PORT);
        return;
    }
    
    // Read the mouse data byte
    uint8_t data = inb(PS2_DATA_PORT);
    
    // Store the byte in our buffer
    packet_buffer[packet_index] = data;
    packet_index++;
    
    // If we have received all 3 bytes of a mouse packet
    if (packet_index >= 3) {
        packet_index = 0;
        
        // Parse the mouse packet
        uint8_t flags = packet_buffer[0];
        int8_t delta_x = packet_buffer[1];
        int8_t delta_y = packet_buffer[2];
        
        // Check if the packet is valid (bit 3 should always be set)
        if (!(flags & PS2_MOUSE_ALWAYS_1)) {
            kprintf("PS/2 Mouse: Invalid packet (flags=0x%02X)\n", flags);
            // Reset packet index to resync
            packet_index = 0;
            return;
        }
        
        // Check for overflow
        if (flags & (PS2_MOUSE_X_OVERFLOW | PS2_MOUSE_Y_OVERFLOW)) {
            kprintf("PS/2 Mouse: Overflow detected\n");
            return;
        }
        
        // Apply sign extension if needed
        if (flags & PS2_MOUSE_X_SIGN) {
            delta_x |= 0xFFFFFF00;
        }
        if (flags & PS2_MOUSE_Y_SIGN) {
            delta_y |= 0xFFFFFF00;
        }
        
        // Y coordinate is inverted in PS/2 mouse
        delta_y = -delta_y;
        
        // Update mouse state
        cursor_X += delta_x;
        cursor_Y += delta_y;
        bool left_button = (flags & PS2_MOUSE_LEFT_BTN) ? true : false;
        bool right_button = (flags & PS2_MOUSE_RIGHT_BTN) ? true : false;
        bool middle_button = (flags & PS2_MOUSE_MIDDLE_BTN) ? true : false;
        bool packet_ready = true;
        
        // Debug output
        kprintf("Mouse: X=%d, Y=%d, L=%d, R=%d, M=%d (dx=%d, dy=%d)\n",
                cursor_X, cursor_Y,
                left_button, right_button, middle_button,
                delta_x, delta_y);
    }
}

Driver ps2mouse_driver = {
    .name = "PS/2 Mouse Driver",
    .init = ps2mouse_init,
    .enable = ps2mouse_enable,
    .disable = ps2mouse_disable,
    .enabled = ps2mouse_enabled,
    .type = DRIVER_TYPE_HID
};