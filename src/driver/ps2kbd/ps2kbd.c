#include <driver/Driver.h>
#include <io.h>
#include <hal/irqController.h>
#include <buffer.h>
#include <keyboard/Keyboard.h>
#include <list.h>
#include <stream/InputStream.h>
#include <intel86.h>
#include <memory/memory.h>
#include <stream/OutputStream.h>
#include <driver/ps2controller/ps2controller.h>

// PS/2 Klavye Komutları (bunlar klavyeye özel, controller'a değil)
#define PS2_KBD_CMD_RESET           0xFF
#define PS2_KBD_CMD_ENABLE          0xF4
#define PS2_KBD_CMD_DISABLE         0xF5
#define PS2_KBD_CMD_SET_SCANCODE    0xF0

// PS/2 Response Kodları
#define PS2_RESPONSE_ACK            0xFA
#define PS2_RESPONSE_RESEND         0xFE
#define PS2_RESPONSE_SELF_TEST_OK   0xAA

extern List* keyboardInputStreamList; // Global list to hold keyboard input streams

Buffer* ps2_event_buffer = NULL; // Buffer for PS/2 keyboard events

static void ps2kbd_init(void);
static void ps2kbd_enable(void);
static void ps2kbd_disable(void);

static int ps2kbd_stream_open();
static void ps2kbd_stream_close();
static int ps2kbd_stream_readChar(char* c);
static int ps2kbd_stream_readString(char* str, size_t maxLength);
static int ps2kbd_stream_readBuffer(void* buffer, size_t size);
static int ps2kbd_stream_available();
static char ps2kbd_stream_peek();
static void ps2kbd_stream_flush();
static bool ps2_kbd_send_command(uint8_t command);

InputStream ps2kbdInputStream = {
    .Open = ps2kbd_stream_open,
    .Close = ps2kbd_stream_close,
    .readChar = ps2kbd_stream_readChar,
    .readString = ps2kbd_stream_readString,
    .readBuffer = ps2kbd_stream_readBuffer,
    .available = ps2kbd_stream_available,
    .peek = ps2kbd_stream_peek,
    .flush = ps2kbd_stream_flush
};

Driver ps2kbd_driver = {
    .name = "PS/2 Keyboard Driver",
    .enabled = false,
    .type = 0,
    .init = ps2kbd_init,
    .enable = ps2kbd_enable,
    .disable = ps2kbd_disable
};

extern void ps2kbd_isr(void); // PS/2 keyboard interrupt service routine
void ps2kbd_handle();

extern bool __kbd_abstraction_initialized; // Flag to check if keyboard abstraction layer is initialized

static void ps2kbd_init(void) {

    // Check for ps2controller initalized
    if (!ps2_controller_initialized) ps2_controller_init();

    // Check for keyboard abstraction layer initialized
    if (!__kbd_abstraction_initialized) {
        if (keyboardInputStream.Open) keyboardInputStream.Open();
        else return;
    }

    // Initialize the PS/2 keyboard driver
    ps2_event_buffer = buffer_create(sizeof(KeyboardKeyEventData));

    if (!ps2_event_buffer) {
        currentOutputStream->printf("Failed to create PS/2 keyboard event buffer.\n");
        return;
    }

    // Register the PS/2 keyboard input stream
    if (keyboardInputStreamList == NULL) {
        keyboardInputStreamList = list_create();
    }

    currentOutputStream->printf("Initializing PS/2 keyboard...\n");

    // Önce ortak controller'ın başlatıldığından emin ol
    if (!ps2_controller_initialized) {
        if (!ps2_controller_init()) {
            currentOutputStream->printf("PS/2 Keyboard: Controller init failed!\n");
            return;
        }
    }

    // Controller'ın hazır olduğundan emin ol
    ps2_controller_flush_buffer();

    // Port 1'in aktif olduğundan emin ol
    ps2_controller_write_command(PS2_CMD_ENABLE_PORT1);

    // Klavyeyi disable et (konfigürasyon için)
    if (!ps2_kbd_send_command(PS2_KBD_CMD_DISABLE)) {
        currentOutputStream->printf("Failed to disable PS/2 keyboard.\n");
        return;
    }

    // Klavyeyi reset et
    currentOutputStream->printf("Resetting PS/2 keyboard...\n");
    if (!ps2_kbd_send_command(PS2_KBD_CMD_RESET)) {
        currentOutputStream->printf("Failed to reset PS/2 keyboard.\n");
        return;
    }

    // Self-test sonucunu bekle
    uint8_t self_test = ps2_controller_read_data();
    if (self_test != PS2_RESPONSE_SELF_TEST_OK) {
        currentOutputStream->printf("PS/2 keyboard self-test failed: 0x%02X\n", self_test);
        return;
    }

    // Scancode set 2'ye ayarla
    currentOutputStream->printf("Setting scancode set 2...\n");
    if (!ps2_kbd_send_command(PS2_KBD_CMD_SET_SCANCODE)) {
        currentOutputStream->printf("Failed to send scancode set command.\n");
        return;
    }
    
    if (!ps2_kbd_send_command(0x02)) { // Scancode set 2
        currentOutputStream->printf("Failed to set scancode set 2.\n");
        return;
    }

    // Scancode set'i doğrula
    if (ps2_kbd_send_command(PS2_KBD_CMD_SET_SCANCODE)) {
        if (ps2_kbd_send_command(0x00)) { // Current scancode set'i oku
            uint8_t current_set = ps2_controller_read_data();
            if (current_set == 0x02) {
                currentOutputStream->printf("Scancode set 2 confirmed.\n");
            } else {
                currentOutputStream->printf("Warning: Scancode set may not be 2 (got: 0x%02X)\n", current_set);
            }
        }
    }

    // Klavyeyi enable et
    if (!ps2_kbd_send_command(PS2_KBD_CMD_ENABLE)) {
        currentOutputStream->printf("Failed to enable PS/2 keyboard.\n");
        return;
    }

    // Konfigürasyonu güncelle - klavye interrupt'ını etkinleştir
    uint8_t config = ps2_controller_get_config();
    config |= PS2_CONFIG_PORT1_INT;    // Klavye interrupt'ı etkinleştir
    config &= ~PS2_CONFIG_PORT1_TRANS; // Translation'ı devre dışı bırak (raw scancode)
    config &= ~PS2_CONFIG_PORT1_CLOCK; // Klavye clock'u etkinleştir
    ps2_controller_set_config(config);

    // IDT'ye interrupt handler'ı ekle (IRQ1 = INT 33)
    intel86_idt_set_entry(33, (uint32_t)ps2kbd_isr, 0x08, 0x8E);

    // PIC'de IRQ1'i unmask et
    irq_controller->enable_irq(1);

    currentOutputStream->printf("PS/2 keyboard driver initialized successfully.\n");
}

void ps2kbd_enable(void) {
    if (ps2_event_buffer == NULL) {
        return;
    }

    if (keyboardInputStreamList == NULL) {
        return;
    }

    if (ps2kbd_driver.enabled) {
        return;
    }

    list_add(keyboardInputStreamList, &ps2kbdInputStream);
    ps2kbd_driver.enabled = true;
    currentOutputStream->printf("PS/2 keyboard driver enabled.\n");
}

void ps2kbd_disable(void) {
    ps2kbd_driver.enabled = false;

    if (keyboardInputStreamList != NULL) {
        list_remove(keyboardInputStreamList, &ps2kbdInputStream);
    }

    buffer_clear(ps2_event_buffer);
}

extern KeyboardLayouts currentLayout;
extern void __ps2kbd_us_qwerty_handle(uint8_t scancode);
extern void __ps2kbd_tr_qwerty_handle(uint8_t scancode);
extern void __ps2kbd_tr_f_handle(uint8_t scancode);

void ps2kbd_handler() {

    if (!ps2_event_buffer) {
        return;
    }

    char scancode = inb(PS2_DATA_PORT);

    if (currentLayout == LAYOUT_US_QWERTY) {
        __ps2kbd_us_qwerty_handle(scancode);
    } else if (currentLayout == LAYOUT_TR_QWERTY) {
        __ps2kbd_tr_qwerty_handle(scancode);
    } else if (currentLayout == LAYOUT_TR_F) {
        __ps2kbd_tr_f_handle(scancode);
    } else {
        return;
    }
}

static int ps2kbd_stream_open() {
    return 0;
}

static void ps2kbd_stream_close() {
}

static int ps2kbd_stream_readChar(char* c) {
    if (!c || !ps2_event_buffer) {
        return -1;
    }

    if (buffer_count(ps2_event_buffer) == 0) {
        return -1;
    }

    KeyboardKeyEventData* event;

    do {
        event = (KeyboardKeyEventData*)buffer_pop(ps2_event_buffer);
        if (!event) {
            return -1;
        }
    } while (event->isPressed == false || event->key == KEY_UNKNOWN);

    *c = event->ascii;
    kfree(event);
    return 1;
}

static int ps2kbd_stream_readString(char* str, size_t maxLength) {
    if (!str || maxLength == 0 || !ps2_event_buffer) {
        return -1;
    }

    size_t length = 0;

    while (length < maxLength - 1) {
        char c;
        if (ps2kbd_stream_readChar(&c) != 0) {
            break;
        }
        str[length++] = c;
    }

    str[length] = '\0';
    return length;
}

static int ps2kbd_stream_readBuffer(void* buffer, size_t size) {
    if (!buffer || size == 0 || !ps2_event_buffer) {
        return -1;
    }

    char* charBuffer = (char*)buffer;
    size_t bytesRead = 0;

    while (bytesRead < size) {
        char c;
        if (ps2kbd_stream_readChar(&c) != 0) {
            break;
        }
        charBuffer[bytesRead++] = c;
    }

    return bytesRead;
}

static int ps2kbd_stream_available() {
    if (!ps2_event_buffer) {
        return 0;
    }
    return buffer_count(ps2_event_buffer);
}

static char ps2kbd_stream_peek() {
    if (!ps2_event_buffer || buffer_is_empty(ps2_event_buffer)) {
        return '\0';
    }

    KeyboardKeyEventData* event = (KeyboardKeyEventData*)buffer_peek(ps2_event_buffer);
    if (!event || !event->isPressed) {
        return '\0';
    }

    return event->ascii;
}

static void ps2kbd_stream_flush() {
}

// PS/2 klavyeye komut gönder ve ACK bekle
static bool ps2_kbd_send_command(uint8_t command) {
    for (int retry = 0; retry < 3; retry++) {
        if (!ps2_controller_wait_write()) {
            continue;
        }
        
        outb(PS2_DATA_PORT, command);
        
        // Response bekle
        uint8_t response = ps2_controller_read_data();
        
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