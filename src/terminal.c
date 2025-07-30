#include <terminal.h>
#include <stdarg.h>
#include <stream/VPrintf.h>
#include <boot/multiboot2.h>
#include <memory/memory.h>
#include <list.h>

gfx_font* terminalFont = &gfx_font8x16;
List* terminalLinePixels = NULL;

gfx_buffer* terminalBuffer = NULL;

gfx_color terminalBackgroundColor = { .argb = 0x00000000 }; // Default to transparent black
gfx_color terminalTextColor = { .argb = 0xFFFFFFFF }; // Default to white

static size_t terminalCursorPosition = 0; // Cursor position
static size_t terminalScrollPosition = 0; // Scroll position

static gfx_size terminalSize;

static size_t invalidateCount = 0; // Count of invalidations

static void terminalUpdateBuffer() {
    if (!terminalBuffer || !terminalBuffer->buffer) return;

    // Clear the terminal buffer
    size_t pixelCount = terminalSize.width * terminalSize.height;
    gfx_color* pixels = (gfx_color*)terminalBuffer->buffer;

    for (size_t i = 0; i < pixelCount; i++) {
        pixels[i] = terminalBackgroundColor;
    }

    // Draw the text lines
    if (terminalLinePixels) {
        size_t lineCount = list_size(terminalLinePixels);
        for (size_t i = terminalScrollPosition; i < lineCount; i++) {
            void* linePixels = list_get(terminalLinePixels, i);
            if (!linePixels) continue;

            gfx_color* pixelLine = (gfx_color*)linePixels;
            for (size_t j = 0; j < terminalSize.width; j++) {
                size_t pixelIndex = (i - terminalScrollPosition) * terminalSize.width + j;
                if (pixelIndex < pixelCount) {
                    pixels[pixelIndex] = pixelLine[j];
                }
            }
        }
    }
}

static void terminalAddLine() {
    if (!terminalLinePixels) return;
    for (size_t i = 0; i < terminalFont->size.height; i++) {
        void* pixelLine = kmalloc(terminalSize.width * sizeof(gfx_color));
        list_add(terminalLinePixels, pixelLine);
    }
}

static size_t terminalLineCount() {
    if (!terminalLinePixels) return 0;
    return list_size(terminalLinePixels) / terminalFont->size.height;
}

static void terminal_open() {

    terminalSize.width = mb2_framebuffer->framebuffer_width;
    terminalSize.height = mb2_framebuffer->framebuffer_height;

    if (!terminalBuffer) {
        terminalBuffer = (gfx_buffer*)kmalloc(sizeof(gfx_buffer));
        if (!terminalBuffer) {
            return; // Memory allocation failed
        }
        terminalBuffer->size = terminalSize;
        terminalBuffer->bpp = mb2_framebuffer->framebuffer_bpp;
        terminalBuffer->buffer = kmalloc(terminalSize.width * terminalSize.height * (terminalBuffer->bpp / 8));
    }
    
    if (terminalLinePixels) {
        list_destroy(terminalLinePixels);
    }

    terminalLinePixels = (List*)kmalloc(sizeof(List));

    if (!terminalLinePixels) {
        return; // Memory allocation failed
    }

    for (size_t i = 0; i < terminalSize.width / terminalFont->size.width; i++) {
        terminalAddLine();
    }

}

static void terminal_close() {

}

static int terminal_write_char(char c) {
    
}

static int terminal_write_string(const char* str) {
    if (!str) return 0;

    int count = 0;
    while (*str) {
        count += terminal_write_char(*str);
        str++;
    }

    return count;
}

static int terminal_write_buffer(const void* buffer, size_t size) {
    if (!buffer || size == 0) return 0;

    const char* data = (const char*)buffer;
    int count = 0;

    for (size_t i = 0; i < size; i++) {
        count += terminal_write_char(data[i]);
    }

    return count;
}

static int terminal_print(const char* str) {
    return terminal_write_string(str);
}

static void terminal_write_char_wrapper(char c) {
    terminal_write_char(c);
}

static int terminal_printf(const char* format, ...) {
    if (!format) return 0;

    va_list args;
    va_start(args, format);
    int count = vprintf(terminal_write_char_wrapper, format, args);
    va_end(args);

    return count;
}

static void terminalUpdatePeriodic() {
    if (invalidateCount == 0) return;
    invalidateCount = 0;

    gfx_copy(terminalBuffer, screen_buffer, (gfx_point){0, 0}, (gfx_point){0, 0}, terminalSize);

}

OutputStream terminalOutputStream = {
    .Open = terminal_open,
    .Close = terminal_close,
    .writeChar = terminal_write_char,
    .writeString = terminal_write_string,
    .writeBuffer = terminal_write_buffer,
    .print = terminal_print,
    .printf = terminal_printf,
};