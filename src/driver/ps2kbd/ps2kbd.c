#include <driver/Driver.h>
#include <io.h>
#include <buffer.h>
#include <keyboard/Keyboard.h>
#include <list.h>
#include <stream/InputStream.h>
#include <intel86.h>
#include <memory/memory.h>
#include <stream/OutputStream.h>

// PS/2 Controller ve Klavye Portları
#define PS2_DATA_PORT    0x60
#define PS2_COMMAND_PORT 0x64

// PS/2 Controller Komutları
#define PS2_CMD_READ_CONFIG     0x20
#define PS2_CMD_WRITE_CONFIG    0x60
#define PS2_CMD_ENABLE_PORT1    0xAE
#define PS2_CMD_TEST_PORT1      0xAB

// PS/2 Klavye Komutları
#define PS2_KBD_CMD_RESET           0xFF
#define PS2_KBD_CMD_ENABLE          0xF4
#define PS2_KBD_CMD_DISABLE         0xF5
#define PS2_KBD_CMD_SET_SCANCODE    0xF0

// PS/2 Response Kodları
#define PS2_RESPONSE_ACK            0xFA
#define PS2_RESPONSE_RESEND         0xFE
#define PS2_RESPONSE_SELF_TEST_OK   0xAA

#define PS2_TIMEOUT 100000

extern List* keyboardInputStreamList; // Global list to hold keyboard input streams

Buffer* ps2_event_buffer = NULL; // Buffer for PS/2 keyboard events

static void ps2kbd_init(void);

static void ps2kbd_enable(void);

static void ps2kbd_disable(void);

static void ps2kbd_stream_open();
static void ps2kbd_stream_close();
static int ps2kbd_stream_readChar(char* c);
static int ps2kbd_stream_readString(char* str, size_t maxLength);
static int ps2kbd_stream_readBuffer(void* buffer, size_t size);
static int ps2kbd_stream_available();
static char ps2kbd_stream_peek();
static void ps2kbd_stream_flush();
static bool ps2_kbd_send_command(uint8_t command);
static bool ps2_send_command(uint8_t command);
static uint8_t ps2_read_data(void);
static bool ps2_wait_write_ready(void);
static bool ps2_wait_read_ready(void);

InputStream ps2kbdInputStream = {
    .Open = ps2kbd_stream_open, // Define Open function
    .Close = ps2kbd_stream_close, // Define Close function
    .readChar = ps2kbd_stream_readChar, // Define readChar function
    .readString = ps2kbd_stream_readString, // Define readString function
    .readBuffer = ps2kbd_stream_readBuffer, // Define readBuffer function
    .available = ps2kbd_stream_available, // Define available function
    .peek = ps2kbd_stream_peek, // Define peek function
    .flush = ps2kbd_stream_flush // Define flush function
};

Driver ps2kbd_driver = {
    .name = "PS/2 Keyboard Driver",
    .enabled = false,
    .type = 0, // Define appropriate type
    .init = ps2kbd_init,
    .enable = ps2kbd_enable,
    .disable = ps2kbd_disable
};

extern void ps2kbd_isr(void); // PS/2 keyboard interrupt service routine

void ps2kbd_handle();

extern bool __kbd_abstraction_initialized; // Flag to check if keyboard abstraction layer is initialized

static void ps2kbd_init(void) {
    // Check for keyboard abstraction layer initialized
    if (!__kbd_abstraction_initialized) {
        if (keyboardInputStream.Open) keyboardInputStream.Open(); // Open the keyboard input stream
        else return; // If Open function is not defined, return
    }

    // Initialize the PS/2 keyboard driver
    ps2_event_buffer = buffer_create(sizeof(KeyboardKeyEventData)); // Create a buffer for scancodes

    if (!ps2_event_buffer) {
        currentOutputStream->printf("Failed to create PS/2 keyboard event buffer.\n");
        return; // Failed to create buffer
    }

    // Register the PS/2 keyboard input stream
    if (keyboardInputStreamList == NULL) {
        keyboardInputStreamList = list_create();
    }

    // PS/2 Klavye Initialization - Mouse'u etkilemeden
    currentOutputStream->printf("Initializing PS/2 keyboard (preserving mouse)...\n");

    // 1. Mevcut output buffer'ı temizle (sadece port 1 için)
    int clear_attempts = 0;
    while ((inb(PS2_COMMAND_PORT) & 0x01) && clear_attempts < 10) {
        uint8_t data = inb(PS2_DATA_PORT);
        (void)data; // Suppress unused variable warning
        clear_attempts++;
        io_wait();
    }

    // 2. Port 1'in aktif olduğundan emin ol (mouse portunu etkilemez)
    if (!ps2_send_command(PS2_CMD_ENABLE_PORT1)) {
        currentOutputStream->printf("Failed to enable PS/2 port 1.\n");
        return;
    }

    // 3. Controller konfigürasyonunu oku
    if (!ps2_send_command(PS2_CMD_READ_CONFIG)) {
        currentOutputStream->printf("Failed to read PS/2 controller config.\n");
        return;
    }
    
    uint8_t config = ps2_read_data();
    
    // 4. Sadece port 1 interrupt'ını enable et, diğer ayarları koru
    config |= 0x01;  // Port 1 interrupt enable
    config &= ~0x40; // Translation disable (raw scancode)
    // Port 2 (mouse) ayarlarını koruyoruz
    
    // 5. Konfigürasyonu geri yaz
    if (!ps2_send_command(PS2_CMD_WRITE_CONFIG)) {
        currentOutputStream->printf("Failed to write PS/2 controller config.\n");
        return;
    }
    
    if (!ps2_wait_write_ready()) {
        currentOutputStream->printf("PS/2 controller not ready for config write.\n");
        return;
    }
    outb(PS2_DATA_PORT, config);

    // 6. Klavyeyi disable et (konfigürasyon için)
    if (!ps2_kbd_send_command(PS2_KBD_CMD_DISABLE)) {
        currentOutputStream->printf("Failed to disable PS/2 keyboard.\n");
        return;
    }

    // 7. Klavyeyi reset et (sadece klavye, mouse'a dokunmaz)
    currentOutputStream->printf("Resetting PS/2 keyboard...\n");
    if (!ps2_kbd_send_command(PS2_KBD_CMD_RESET)) {
        currentOutputStream->printf("Failed to reset PS/2 keyboard.\n");
        return;
    }

    // Self-test sonucunu bekle
    uint8_t self_test = ps2_read_data();
    if (self_test != PS2_RESPONSE_SELF_TEST_OK) {
        currentOutputStream->printf("PS/2 keyboard self-test failed: 0x%02X\n", self_test);
        return;
    }

    // 8. Scancode set 2'ye ayarla
    currentOutputStream->printf("Setting scancode set 2...\n");
    if (!ps2_kbd_send_command(PS2_KBD_CMD_SET_SCANCODE)) {
        currentOutputStream->printf("Failed to send scancode set command.\n");
        return;
    }
    
    if (!ps2_kbd_send_command(0x02)) { // Scancode set 2
        currentOutputStream->printf("Failed to set scancode set 2.\n");
        return;
    }

    // 9. Scancode set'i doğrula (opsiyonel)
    if (ps2_kbd_send_command(PS2_KBD_CMD_SET_SCANCODE)) {
        if (ps2_kbd_send_command(0x00)) { // Current scancode set'i oku
            uint8_t current_set = ps2_read_data();
            if (current_set == 0x02) {
                currentOutputStream->printf("Scancode set 2 confirmed.\n");
            } else {
                currentOutputStream->printf("Warning: Scancode set may not be 2 (got: 0x%02X)\n", current_set);
            }
        }
    }

    // 10. Klavyeyi enable et
    if (!ps2_kbd_send_command(PS2_KBD_CMD_ENABLE)) {
        currentOutputStream->printf("Failed to enable PS/2 keyboard.\n");
        return;
    }

    // 11. IDT'ye interrupt handler'ı ekle (IRQ1 = INT 33)
    intel86_idt_set_entry(33, (uint32_t)ps2kbd_isr, 0x08, 0x8E);

    // 12. PIC'de IRQ1'i unmask et
    pic_unmask(1);

    currentOutputStream->printf("PS/2 keyboard driver initialized successfully.\n");
}

void ps2kbd_enable(void) {

    if (ps2_event_buffer == NULL) {
        return; // Buffer not initialized
    }

    if (keyboardInputStreamList == NULL) {
        return; // Keyboard input stream list not initialized
    }

    if (ps2kbd_driver.enabled) {
        return; // Already enabled
    }

    // Enable the PS/2 keyboard driver

    list_add(keyboardInputStreamList, &ps2kbdInputStream);

    ps2kbd_driver.enabled = true;

    currentOutputStream->printf("PS/2 keyboard driver enabled.\n");

}

void ps2kbd_disable(void) {
    // Disable the PS/2 keyboard driver
    ps2kbd_driver.enabled = false;

    // Additional disabling logic can be added here
    if (keyboardInputStreamList != NULL) {
        list_remove(keyboardInputStreamList, &ps2kbdInputStream); // Remove the PS/2 keyboard input stream
    }

    buffer_clear(ps2_event_buffer); // Clear the PS/2 event buffer
}

extern KeyboardLayouts currentLayout; // Current keyboard layout

extern void __ps2kbd_us_qwerty_handle(uint8_t scancode);
extern void __ps2kbd_tr_qwerty_handle(uint8_t scancode);
extern void __ps2kbd_tr_f_handle(uint8_t scancode);

void ps2kbd_handler() {
    // Handle PS/2 keyboard events
    if (!ps2_event_buffer) {
        return; // No buffer
    }

    char scancode = inb(0x60); // Read scancode from PS/2 keyboard

    if (currentLayout == LAYOUT_US_QWERTY) {
        __ps2kbd_us_qwerty_handle(scancode); // Handle US QWERTY layout
    } else if (currentLayout == LAYOUT_TR_QWERTY) {
        __ps2kbd_tr_qwerty_handle(scancode); // Handle Turkish QWERTY layout
    } else if (currentLayout == LAYOUT_TR_F) {
        __ps2kbd_tr_f_handle(scancode); // Handle Turkish F layout
    } else {
        // Unknown layout, do nothing or handle error
        return;
    }

}

static void ps2kbd_stream_open() {

}

static void ps2kbd_stream_close() {
    // Close the PS/2 keyboard input stream
    
}

static int ps2kbd_stream_readChar(char* c) {
    // Read a single character from the PS/2 keyboard input stream

    if (!c || !ps2_event_buffer) {
        return -1; // Invalid pointer or buffer not initialized
    }

    if (list_size(ps2_event_buffer) == 0) {
        return -1; // No events to read
    }

    KeyboardKeyEventData* event;

    do {
        event = (KeyboardKeyEventData*)buffer_pop(ps2_event_buffer); // Pop the event from the buffer
        if (!event) {
            return -1; // Failed to pop event
        }
    } while (event->isPressed == false || event->key == KEY_UNKNOWN); // Skip released keys && unknown keys

    *c = event->ascii; // Set the character to the ASCII value of the event

    kfree(event); // Free the event memory

    return 1; // Success
}

static int ps2kbd_stream_readString(char* str, size_t maxLength) {
    // Read a string from the PS/2 keyboard input stream

    if (!str || maxLength == 0 || !ps2_event_buffer) {
        return -1; // Invalid pointer or buffer not initialized
    }

    size_t length = 0;

    while (length < maxLength - 1) { // Leave space for null terminator
        char c;
        if (ps2kbd_stream_readChar(&c) != 0) {
            break; // No more characters to read
        }
        str[length++] = c; // Add character to string
    }

    str[length] = '\0'; // Null-terminate the string

    return length; // Return the length of the string read
}

static int ps2kbd_stream_readBuffer(void* buffer, size_t size) {
    // Read a buffer from the PS/2 keyboard input stream

    if (!buffer || size == 0 || !ps2_event_buffer) {
        return -1; // Invalid pointer or buffer not initialized
    }

    char* charBuffer = (char*)buffer;
    size_t bytesRead = 0;

    while (bytesRead < size) {
        char c;
        if (ps2kbd_stream_readChar(&c) != 0) {
            break; // No more characters to read
        }
        charBuffer[bytesRead++] = c; // Add character to buffer
    }

    return bytesRead; // Return the number of bytes read
}

static int ps2kbd_stream_available() {
    // Check if there are characters available to read from the PS/2 keyboard input stream

    if (!ps2_event_buffer) {
        return 0; // Buffer not initialized
    }

    return buffer_count(ps2_event_buffer); // Return the number of events in the buffer
}

static char ps2kbd_stream_peek() {
    // Peek at the next character in the PS/2 keyboard input stream

    if (!ps2_event_buffer || buffer_is_empty(ps2_event_buffer)) {
        return '\0'; // No characters available
    }

    KeyboardKeyEventData* event = (KeyboardKeyEventData*)buffer_peek(ps2_event_buffer); // Peek at the next event
    if (!event || !event->isPressed) {
        return '\0'; // No pressed key available
    }

    return event->ascii; // Return the ASCII character of the event
}

static void ps2kbd_stream_flush() {

}

// PS/2 controller'ın hazır olmasını bekle (okuma için)
static bool ps2_wait_read_ready(void) {
    for (int i = 0; i < PS2_TIMEOUT; i++) {
        if (inb(PS2_COMMAND_PORT) & 0x01) {
            return true;
        }
        io_wait();
    }
    return false;
}

// PS/2 controller'ın hazır olmasını bekle (yazma için)
static bool ps2_wait_write_ready(void) {
    for (int i = 0; i < PS2_TIMEOUT; i++) {
        if (!(inb(PS2_COMMAND_PORT) & 0x02)) {
            return true;
        }
        io_wait();
    }
    return false;
}

// PS/2 controller'dan veri oku
static uint8_t ps2_read_data(void) {
    if (!ps2_wait_read_ready()) {
        return 0xFF; // Timeout
    }
    return inb(PS2_DATA_PORT);
}

// PS/2 controller'a komut gönder
static bool ps2_send_command(uint8_t command) {
    if (!ps2_wait_write_ready()) {
        return false;
    }
    outb(PS2_COMMAND_PORT, command);
    return true;
}

// PS/2 klavyeye komut gönder ve ACK bekle
static bool ps2_kbd_send_command(uint8_t command) {
    for (int retry = 0; retry < 3; retry++) {
        if (!ps2_wait_write_ready()) {
            continue;
        }
        
        outb(PS2_DATA_PORT, command);
        
        // Response bekle
        uint8_t response = ps2_read_data();
        
        if (response == PS2_RESPONSE_ACK) {
            return true;
        } else if (response == PS2_RESPONSE_RESEND) {
            // Yeniden dene
            continue;
        } else {
            // Beklenmeyen response
            return false;
        }
    }
    return false;
}