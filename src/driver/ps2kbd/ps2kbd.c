#include <driver/Driver.h>
#include <io.h>
#include <buffer.h>
#include <keyboard/Keyboard.h>
#include <list.h>
#include <stream/InputStream.h>

extern List* keyboardInputStreamList; // Global list to hold keyboard input streams

Buffer* ps2_scancode_buffer = NULL; // Buffer for PS/2 scancodes

void ps2kbd_init(void);

void ps2kbd_enable(void);

void ps2kbd_disable(void);

InputStream ps2kbdInputStream = {
    .Open = NULL, // Define Open function
    .Close = NULL, // Define Close function
    .readChar = NULL, // Define readChar function
    .readString = NULL, // Define readString function
    .readBuffer = NULL, // Define readBuffer function
    .available = NULL, // Define available function
    .peek = NULL, // Define peek function
    .flush = NULL // Define flush function
};

Driver ps2kbd_driver = {
    .name = "PS/2 Keyboard Driver",
    .enabled = false,
    .type = 0, // Define appropriate type
    .init = ps2kbd_init,
    .enable = ps2kbd_enable,
    .disable = ps2kbd_disable
};
