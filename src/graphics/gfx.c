#include <graphics/gfx.h>
#include <boot/multiboot2.h>
#include <stream/OutputStream.h>
#include <list.h>
#include <memory/memory.h>
#include <task/PeriodicTask.h>

List* gfx_buffers;

gfx_buffer* screen_buffer;
gfx_buffer* hardwareBuffer;

PeriodicTask* gfx_drawBufferTask;

void gfx_draw();

void __attribute__((optimize("O0"))) gfx_init() {
    if (!gfx_buffers) {
        gfx_buffers = list_create();

        if (!gfx_buffers) {
            currentOutputStream->printf("Failed to initialize graphics buffers list.\n");
            return; // Initialization failed
        }

        screen_buffer = gfx_create_buffer((gfx_size){800, 600}); // Default screen size and 32 bpp

    }

    hardwareBuffer = (gfx_buffer*)kmalloc(sizeof(gfx_buffer));

    hardwareBuffer->size.width = mb2_framebuffer->framebuffer_width;
    hardwareBuffer->size.height = mb2_framebuffer->framebuffer_height;
    hardwareBuffer->bpp = mb2_framebuffer->framebuffer_bpp;
    hardwareBuffer->drawBeginLineIndex = 0; // Start drawing from the first line
    hardwareBuffer->buffer = (uint32_t)mb2_framebuffer->framebuffer_addr; // Use the framebuffer address directly

    gfx_drawBufferTask = createPeriodicTask((uint32_t)(uintptr_t)gfx_draw, 100); // 10 FPS
    startPeriodicTask(gfx_drawBufferTask);

}

gfx_buffer* gfx_create_buffer(gfx_size size) {
    gfx_buffer* buffer = (gfx_buffer*)kmalloc(sizeof(gfx_buffer));
    if (!buffer) {
        currentOutputStream->printf("Failed to allocate memory for graphics buffer.\n");
        return NULL; // Memory allocation failed
    }

    buffer->size = size;
    buffer->drawBeginLineIndex = 0; // Default to start drawing from the first line
    buffer->bpp = 32; // Default to 32 bits per pixel
    buffer->buffer = kmalloc(size.width * size.height * (buffer->bpp / 8)); // Allocate pixel buffer

    if (!buffer->buffer) {
        currentOutputStream->printf("Failed to allocate pixel buffer for graphics buffer.\n");
        kfree(buffer);
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
        kfree(buffer->buffer); // Free the pixel buffer
    }

    list_remove(gfx_buffers, buffer); // Remove from the list
    kfree(buffer); // Free the gfx_buffer structure
}

void gfx_draw() {

    if (!gfx_buffers || !screen_buffer) {
        currentOutputStream->printf("[ERROR] Graphics buffers not initialized.\n");
        return; // No buffers to draw
    }

    if (list_size(gfx_buffers) == 0) {
        return; // Nothing to draw
    }

    // Iterate through all buffers and draw them
    ListNode* node = gfx_buffers->head;
    while (node) {
        gfx_buffer* buffer = (gfx_buffer*)node->data;
        if (buffer && buffer->buffer) {
            // Draw the buffer to the screen
            for (size_t y = 0; y < buffer->size.height; y++) {
                for (size_t x = 0; x < buffer->size.width; x++) {
                    VDrawPixel(hardwareBuffer, x, y, *(gfx_color*)(buffer->buffer + (y * buffer->size.width + x) * 4));
                }
            }
        }
        node = node->next;
    }

    // Update the screen with the drawn content
    // This would typically involve a system call or hardware-specific operation

}
