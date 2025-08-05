#include <graphics/gfx.h>
#include <stream/OutputStream.h>
#include <list.h>

List* gfx_buffers;

gfx_buffer* screen_buffer;

void gfx_init() {
    if (!gfx_buffers) {
        gfx_buffers = list_create();

        if (!gfx_buffers) {
            currentOutputStream->printf("Failed to initialize graphics buffers list.\n");
            return; // Initialization failed
        }

        screen_buffer = gfx_create_buffer((gfx_size){800, 600}); // Default screen size and 32 bpp

    }
}

gfx_buffer* gfx_create_buffer(gfx_size size) {
    gfx_buffer* buffer = (gfx_buffer*)malloc(sizeof(gfx_buffer));
    if (!buffer) {
        currentOutputStream->printf("Failed to allocate memory for graphics buffer.\n");
        return NULL; // Memory allocation failed
    }

    buffer->size = size;
    buffer->bpp = 32; // Default to 32 bits per pixel
    buffer->buffer = malloc(size.width * size.height * (buffer->bpp / 8)); // Allocate pixel buffer

    if (!buffer->buffer) {
        currentOutputStream->printf("Failed to allocate pixel buffer for graphics buffer.\n");
        free(buffer);
        return NULL; // Memory allocation failed
    }

    list_add(gfx_buffers, buffer); // Add the new buffer to the list
    return buffer;
}

void gfx_destroy_buffer(gfx_buffer* buffer) {
    if (!buffer) {
        return; // Nothing to destroy
    }

    if (buffer->buffer) {
        free(buffer->buffer); // Free the pixel buffer
    }

    list_remove(gfx_buffers, buffer); // Remove from the list
    free(buffer); // Free the gfx_buffer structure
}
