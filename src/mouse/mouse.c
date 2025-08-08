#include <mouse/mouse.h>
#include <graphics/gfx.h>
#include <graphics/bmp.h>
#include <stream/OutputStream.h>

int cursor_X = 300; // Initialize cursor X position
int cursor_Y = 250; // Initialize cursor Y position

extern gfx_buffer* hardware_buffer; // Pointer to the hardware buffer

extern unsigned char __src_mouse_cursor_21_bmp[];
extern unsigned int __src_mouse_cursor_21_bmp_len;

gfx_bitmap* __cursorBitmap; // Pointer to the cursor bitmap

void __mouse_draw() {
    if (!__cursorBitmap) {
        // Load the cursor bitmap from memory
        __cursorBitmap = bmp_load_from_memory(__src_mouse_cursor_21_bmp, __src_mouse_cursor_21_bmp_len);
        if (!__cursorBitmap) {
            currentOutputStream->printf("Failed to load mouse cursor bitmap: %s\n", bmp_get_error_string(bmp_get_last_error()));
            return;
        }
        currentOutputStream->printf("Mouse cursor bitmap loaded successfully.\n");
    }

    // Draw the cursor bitmap at the current position
    gfx_draw_bitmap(hardware_buffer, cursor_X, cursor_Y, __cursorBitmap->pixels, __cursorBitmap->size.width, __cursorBitmap->size.height);

}
