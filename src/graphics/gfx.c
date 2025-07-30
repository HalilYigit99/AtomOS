#include <graphics/gfx.h>
#include <boot/multiboot2.h>
#include <stddef.h>
#include <memory/memory.h>
#include <stdint.h>
#include <print.h>

gfx_buffer* screen_buffer = NULL;
gfx_font* default_font = &gfx_font8x16;

// Manual abs function to avoid math.h dependency issues
static int gfx_abs(int x) {
    return (x < 0) ? -x : x;
}

void gfx_init()
{
    // Initialize graphics subsystem
    if (!mb2_framebuffer)
    {
        return; // No framebuffer available
    }

    // Get framebuffer information
    uint32_t width = mb2_framebuffer->framebuffer_width;
    uint32_t height = mb2_framebuffer->framebuffer_height;
    uint32_t bpp = mb2_framebuffer->framebuffer_bpp;
    uint64_t framebuffer_addr = mb2_framebuffer->framebuffer_addr;

    // Validate framebuffer
    if (framebuffer_addr == 0 || width == 0 || height == 0) {
        return;
    }

    screen_buffer = (gfx_buffer*)kmalloc(sizeof(gfx_buffer));
    if (!screen_buffer) {
        return; // Memory allocation failed
    }

    screen_buffer->size.width = width;
    screen_buffer->size.height = height;
    screen_buffer->bpp = bpp;
    screen_buffer->buffer = (void*)(uintptr_t)framebuffer_addr;

    // Initialize default font
    default_font = &gfx_font8x8;
}

void gfx_draw_pixel(gfx_buffer* buffer, gfx_point position, gfx_color color)
{
    if (!buffer || !buffer->buffer) {
        return; // Invalid buffer
    }

    if (position.x < 0 || position.x >= (int)buffer->size.width ||
        position.y < 0 || position.y >= (int)buffer->size.height) {
        return; // Out of bounds
    }

    // Calculate pixel address using framebuffer pitch
    uint32_t pitch = mb2_framebuffer->framebuffer_pitch;
    uint32_t bytes_per_pixel = buffer->bpp / 8;
    
    uint32_t offset = position.y * pitch + position.x * bytes_per_pixel;
    uint32_t* pixel = (uint32_t*)((uintptr_t)buffer->buffer + offset);

    // Safety check: ensure we're not writing beyond framebuffer
    uintptr_t fb_start = (uintptr_t)buffer->buffer;
    uintptr_t fb_end = fb_start + (buffer->size.height * pitch);
    uintptr_t pixel_addr = (uintptr_t)pixel;
    
    if (pixel_addr < fb_start || pixel_addr >= fb_end) {
        return;
    }

    *pixel = color.argb;
}

void gfx_draw_line(gfx_buffer* buffer, gfx_line line)
{
    if (!buffer || !buffer->buffer) {
        return; // Invalid buffer
    }

    int dx = gfx_abs(line.end.x - line.start.x);
    int dy = gfx_abs(line.end.y - line.start.y);
    int steps = (dx > dy) ? dx : dy;
    
    if (steps == 0) {
        gfx_draw_pixel(buffer, line.start, line.color);
        return;
    }
    
    // Use integer arithmetic to avoid floating point issues
    int x_inc_scaled = ((line.end.x - line.start.x) * 1000) / steps;
    int y_inc_scaled = ((line.end.y - line.start.y) * 1000) / steps;

    int x_scaled = line.start.x * 1000;
    int y_scaled = line.start.y * 1000;

    for (int i = 0; i <= steps; i++) {
        int x = x_scaled / 1000;
        int y = y_scaled / 1000;
        gfx_draw_pixel(buffer, (gfx_point){x, y}, line.color);
        x_scaled += x_inc_scaled;
        y_scaled += y_inc_scaled;
    }
}

void gfx_draw_rect(gfx_buffer* buffer, gfx_rect rect, gfx_color color)
{
    if (!buffer) {
        return;
    }

    // Top and bottom lines
    for (int x = rect.position.x; x < rect.position.x + (int)rect.size.width; x++) {
        gfx_draw_pixel(buffer, (gfx_point){x, rect.position.y}, color);
        gfx_draw_pixel(buffer, (gfx_point){x, rect.position.y + (int)rect.size.height - 1}, color);
    }

    // Left and right lines
    for (int y = rect.position.y; y < rect.position.y + (int)rect.size.height; y++) {
        gfx_draw_pixel(buffer, (gfx_point){rect.position.x, y}, color);
        gfx_draw_pixel(buffer, (gfx_point){rect.position.x + (int)rect.size.width - 1, y}, color);
    }
}

void gfx_fill_rect(gfx_buffer* buffer, gfx_rect rect, gfx_color color)
{
    if (!buffer) {
        return;
    }

    for (int y = rect.position.y; y < rect.position.y + (int)rect.size.height; y++) {
        for (int x = rect.position.x; x < rect.position.x + (int)rect.size.width; x++) {
            gfx_draw_pixel(buffer, (gfx_point){x, y}, color);
        }
    }
}

void gfx_draw_char(gfx_buffer* buffer, gfx_point position, char c, gfx_color color)
{
    if (!buffer || !default_font) {
        return; // Invalid buffer or no default font loaded
    }

    // Only bitmap fonts supported currently
    if (default_font->type != GFX_FONT_BITMAP) {
        return;
    }

    // Character range check (ASCII 0-127)
    if (c < 0 || c > 127) {
        return; // Character not in font range
    }

    // Get font dimensions
    int char_width = default_font->size.width;
    int char_height = default_font->size.height;
    
    // Get font data pointer correctly
    unsigned char (*font_data)[8] = (unsigned char (*)[8])default_font->glyphs;
    
    // Draw bitmap for each pixel
    for (int y = 0; y < char_height; y++) {
        unsigned char row_data = font_data[c][y];
        for (int x = 0; x < char_width; x++) {
            // Draw pixel if bit is set (starting from LSB for correct orientation)
            if (row_data & (1 << x)) {
                gfx_point pixel_pos = {position.x + x, position.y + y};
                gfx_draw_pixel(buffer, pixel_pos, color);
            }
        }
    }
}

void gfx_draw_text(gfx_buffer* buffer, gfx_point position, const char* text, gfx_color color)
{
    if (!buffer || !text || !default_font) {
        return;
    }

    gfx_point current_pos = position;
    
    while (*text) {
        if (*text == '\n') {
            // New line
            current_pos.x = position.x;
            current_pos.y += default_font->size.height;
        } else if (*text == '\r') {
            // Carriage return
            current_pos.x = position.x;
        } else {
            // Draw character
            gfx_draw_char(buffer, current_pos, *text, color);
            current_pos.x += default_font->size.width;
        }
        text++;
    }
}

void gfx_clear(gfx_buffer* buffer, gfx_color color)
{
    if (!buffer) {
        return;
    }

    gfx_rect full_screen = {
        .size = {buffer->size.width, buffer->size.height}, 
        .position = {0, 0}, 
        .color = color
    };
    gfx_fill_rect(buffer, full_screen, color);
}

void gfx_draw_circle(gfx_buffer* buffer, gfx_circle circle)
{
    if (!buffer || circle.radius.width <= 0) return;
    
    int radius = circle.radius.width;
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;
    
    while (y >= x) {
        // Draw 8 octants
        gfx_draw_pixel(buffer, (gfx_point){circle.position.x + x, circle.position.y + y}, circle.color);
        gfx_draw_pixel(buffer, (gfx_point){circle.position.x - x, circle.position.y + y}, circle.color);
        gfx_draw_pixel(buffer, (gfx_point){circle.position.x + x, circle.position.y - y}, circle.color);
        gfx_draw_pixel(buffer, (gfx_point){circle.position.x - x, circle.position.y - y}, circle.color);
        gfx_draw_pixel(buffer, (gfx_point){circle.position.x + y, circle.position.y + x}, circle.color);
        gfx_draw_pixel(buffer, (gfx_point){circle.position.x - y, circle.position.y + x}, circle.color);
        gfx_draw_pixel(buffer, (gfx_point){circle.position.x + y, circle.position.y - x}, circle.color);
        gfx_draw_pixel(buffer, (gfx_point){circle.position.x - y, circle.position.y - x}, circle.color);
        
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        x++;
    }
}

void gfx_copy(gfx_buffer* src, gfx_buffer* dest, gfx_point src_pos, gfx_point dest_pos, gfx_size size)
{
    if (!src || !dest || !src->buffer || !dest->buffer) return;
    
    for (uint32_t y = 0; y < size.height; y++) {
        for (uint32_t x = 0; x < size.width; x++) {
            gfx_point src_pixel = {src_pos.x + (int)x, src_pos.y + (int)y};
            gfx_point dest_pixel = {dest_pos.x + (int)x, dest_pos.y + (int)y};
            
            // Bounds check
            if (src_pixel.x >= 0 && src_pixel.x < (int)src->size.width &&
                src_pixel.y >= 0 && src_pixel.y < (int)src->size.height &&
                dest_pixel.x >= 0 && dest_pixel.x < (int)dest->size.width &&
                dest_pixel.y >= 0 && dest_pixel.y < (int)dest->size.height) {
                
                // Simple pixel copy - could be optimized
                uint32_t* src_ptr = (uint32_t*)((uintptr_t)src->buffer + 
                                                (src_pixel.y * src->size.width + src_pixel.x) * (src->bpp / 8));
                gfx_color pixel_color = {.argb = *src_ptr};
                gfx_draw_pixel(dest, dest_pixel, pixel_color);
            }
        }
    }
}

void gfx_set_font(gfx_font* font)
{
    if (font) {
        default_font = font;
    }
}

gfx_font* gfx_get_font()
{
    return default_font;
}