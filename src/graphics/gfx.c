#include <graphics/gfx.h>

#include <boot/multiboot2.h>

gfx_buffer* screen_buffer;
gfx_font* default_font;

void gfx_init()
{
    // Initialize graphics subsystem
    // This function can be expanded to set up graphics modes, load fonts, etc.

    if (!mb2_framebuffer)
    {
        return; // No framebuffer available
    }

    // Get framebuffer information
    uint32_t width = mb2_framebuffer->framebuffer_width;
    uint32_t height = mb2_framebuffer->framebuffer_height;
    uint32_t bpp = mb2_framebuffer->framebuffer_bpp;
    uint32_t pitch = mb2_framebuffer->framebuffer_pitch;
    uint64_t framebuffer_addr = mb2_framebuffer->framebuffer_addr;

    screen_buffer = (gfx_buffer*)malloc(sizeof(gfx_buffer));

    if (!screen_buffer)
    {
        return; // Memory allocation failed
    }

    screen_buffer->size.width = width;
    screen_buffer->size.height = height;
    screen_buffer->bpp = bpp;
    screen_buffer->buffer = (void*)(uintptr_t)framebuffer_addr;

}

void gfx_draw_pixel(gfx_buffer* buffer, int x, int y, gfx_color color)
{
    if (!screen_buffer || !screen_buffer->buffer)
    {
        return; // No screen buffer available
    }

    if (x < 0 || x >= screen_buffer->size.width || y < 0 || y >= screen_buffer->size.height)
    {
        return; // Out of bounds
    }

    uint32_t* pixel = (uint32_t*)((uintptr_t)buffer->buffer + y * buffer->size.width * (buffer->bpp / 8) + x * (buffer->bpp / 8));
    *pixel = color.argb; // Assuming ARGB format

}


