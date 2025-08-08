#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <list.h>
#include <graphics/gfx.h>
#include <stream/OutputStream.h>

typedef struct {
    gfx_color background_color; // Background color of the terminal
    gfx_color text_color;       // Text color of the terminal
    size_t cursor_location; // Current cursor position in the terminal buffer
    size_t width;               // Width of the terminal in characters
    size_t height;              // Height of the terminal in characters
    gfx_buffer* terminal_buffer; // Pointer to the terminal's graphics buffer
    List* eventHandlers; // List of event handlers for terminal events
    bool cursor_blink; // Whether the cursor should blink
    bool input_allowed; // Whether input is allowed in the terminal
} Terminal;

Terminal* terminal_create(size_t width, size_t height, gfx_color background_color, gfx_color text_color);
void terminal_destroy(Terminal* terminal);

void terminal_clear(Terminal* terminal);

void terminal_set_cursor(Terminal* terminal, size_t x, size_t y);

void terminal_putChar(Terminal* terminal, char c);
void terminal_putString(Terminal* terminal, const char* str);
void terminal_printf(Terminal* terminal, const char* format, ...);

void terminal_setCursorBlink(Terminal* terminal, bool blink);

void terminal_setInputAllowed(Terminal* terminal, bool allowed);

#ifdef __cplusplus
}
#endif
