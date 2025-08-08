#include <terminal/terminal.h>
#include <boot/multiboot2.h>
#include <keyboard/Keyboard.h>
#include <stream/OutputStream.h>
#include <memory/memory.h>

Terminal* terminal_create(size_t width, size_t height, gfx_color background_color, gfx_color text_color) {
    Terminal* terminal = (Terminal*)kmalloc(sizeof(Terminal));
    if (!terminal) {
        return NULL; // Memory allocation failed
    }

    terminal->background_color = background_color;
    terminal->text_color = text_color;
    terminal->cursor_location = 0;
    terminal->width = width;
    terminal->height = height;
    terminal->terminal_buffer = gfx_create_buffer(width, height);
    terminal->eventHandlers = list_create();
    terminal->cursor_blink = false;
    terminal->input_allowed = true;

    return terminal;
}

void terminal_destroy(Terminal* terminal) {
    if (terminal) {
        if (terminal->terminal_buffer) {
            gfx_screen_unregister_buffer(terminal->terminal_buffer);
            kfree(terminal->terminal_buffer->buffer);
            kfree(terminal->terminal_buffer);
        }
        list_destroy(terminal->eventHandlers);
        kfree(terminal);
    }
}

void terminal_clear(Terminal* terminal) {
    if (terminal && terminal->terminal_buffer) {
        gfx_clear_buffer(terminal->terminal_buffer, terminal->background_color);
        terminal->cursor_location = 0; // Reset cursor position
    }
}
