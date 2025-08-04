#include <terminal/terminal.h>
#include <boot/multiboot2.h>
#include <stream/VPrintf.h>
#include <graphics/gfx.h>
#include <memory/memory.h>
#include <list.h>

extern OutputStream terminal_argb_stream;

gfx_buffer* terminal_argb_buffer;

static gfx_color currentFontColor;
static gfx_color currentBackgroundColor;

static gfx_font* terminalFont = &gfx_font8x16;

static List* terminalLines;

static uint32_t terminalInvalidateCount = 0;

// Index of the first line being displayed
static uint32_t terminalDisplayLineIndex = 0;

static void terminal_scrollUp(size_t lines) {
    if (lines == 0 || !terminalLines) return;

    size_t totalLines = list_size(terminalLines);
    if (totalLines == 0) return;

    size_t newLineIndex;

    if (terminalDisplayLineIndex > lines) newLineIndex = terminalDisplayLineIndex - lines;
    else newLineIndex = 0; // Don't go below zero

    terminalDisplayLineIndex = newLineIndex;

    terminalInvalidateCount++;
}

static void terminal_scrollDown(size_t lines) {
    size_t totalLines = list_size(terminalLines);
    size_t newLineIndex = terminalDisplayLineIndex + lines;

    if (newLineIndex >= totalLines) {
        newLineIndex = totalLines - 1; // Don't exceed the number of lines
    }

    terminalDisplayLineIndex = newLineIndex;

    terminalInvalidateCount++;

}

static void terminal_addLine() {

    if (!terminalLines) return;

    void* newLine = kmalloc(terminal_argb_buffer->size.width * terminal_argb_buffer->size.height * (terminal_argb_buffer->bpp / 8));

    if (!newLine) {
        currentOutputStream->printf("Failed to allocate memory for new terminal line.\n");
        return;
    }

    list_add(terminalLines, newLine);

}

static void terminal_drawPixel(size_t x, size_t y, gfx_color color) {
    if (!terminal_argb_buffer || !terminal_argb_buffer->buffer) {
        return; // Invalid buffer
    }

    if (x < 0 || x >= (int)terminal_argb_buffer->size.width) {
        return; // Out of bounds
    }

    if (y < list_size(terminalLines)) {
        return; // Out of bounds
    }

    uint32_t* line = list_get(terminalLines, y);

    line[x] = color.argb; // Assuming ARGB format

}

static void terminal_drawChar(size_t x, size_t y, char c) {
    if (!terminal_argb_buffer || !terminal_argb_buffer->buffer) {
        return; // Invalid buffer
    }

    gfx_font* font = terminalFont;

    

}

void terminal_argb_open() {
    if (!mb2_framebuffer || (mb2_framebuffer->framebuffer_bpp != 32)) {
        currentOutputStream->printf("Supported framebuffer not found.\n");
        return;
    }

    currentOutputStream->printf("Opening ARGB Terminal...\n");

    terminal_argb_buffer = gfx_create_buffer(mb2_framebuffer->framebuffer_width, mb2_framebuffer->framebuffer_height);

    if (!terminal_argb_buffer) {
        currentOutputStream->printf("Failed to create ARGB terminal buffer.\n");
        return;
    }

    if (!terminalLines) {
        terminalLines = list_create();
        if (!terminalLines) {
            currentOutputStream->printf("Failed to create terminal buffer list.\n");
            return;
        }
    }



}

void terminal_argb_close() {

}

int terminal_argb_writeChar(char c) {
    
}

int terminal_argb_writeString(const char* str) {
    while (*str) {
        terminal_argb_writeChar(*str++);
    }
    return 0; // Assuming success
}

int terminal_argb_writeBuffer(const void* buffer, size_t size) {
    const char* data = (const char*)buffer;
    for (size_t i = 0; i < size; i++) {
        terminal_argb_writeChar(data[i]);
    }
    return 0; // Assuming success
}

int terminal_argb_print(const char* str) {
    return terminal_argb_writeString(str);
}

int terminal_argb_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vprintf(terminal_argb_writeString, format, args);
    va_end(args);
    return result;
}


OutputStream terminal_argb_stream = {
    .Open = terminal_argb_open,
    .Close = terminal_argb_close,
    .writeChar = terminal_argb_writeChar,
    .writeString = terminal_argb_writeString,
    .writeBuffer = terminal_argb_writeBuffer,
    .print = terminal_argb_print,
    .printf = terminal_argb_printf
};