#include <graphics/gfx.h>
#include <boot/multiboot2.h>
#include <stddef.h>
#include <memory/memory.h>
#include <stdint.h>
#include <math.h>
#include <print.h>

gfx_buffer* screen_buffer;
gfx_font* default_font = &gfx_font8x16; // Default font set to 8x8 bitmap font

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
    uint32_t pitch = mb2_framebuffer->framebuffer_pitch;
    uint64_t framebuffer_addr = mb2_framebuffer->framebuffer_addr;

    screen_buffer = (gfx_buffer*)kmalloc(sizeof(gfx_buffer));

    if (!screen_buffer)
    {
        kprintf("Failed to allocate memory for screen buffer\n");
        return; // Memory allocation failed
    }

    screen_buffer->size.width = width;
    screen_buffer->size.height = height;
    screen_buffer->bpp = bpp;
    screen_buffer->buffer = (void*)(uintptr_t)framebuffer_addr;

    // Initialize default font (bu kısım font dosyası yüklendikten sonra ayarlanacak)
    default_font = NULL;
}

void gfx_draw_pixel(gfx_buffer* buffer, gfx_point position, gfx_color color)
{
    if (!buffer || !buffer->buffer)
    {
        return; // Invalid buffer
    }

    if (position.x < 0 || position.x >= buffer->size.width ||
        position.y < 0 || position.y >= buffer->size.height)
    {
        return; // Out of bounds
    }

    uint32_t* pixel = (uint32_t*)((uintptr_t)buffer->buffer + 
                                  (position.y * buffer->size.width + position.x) * (buffer->bpp / 8));

    *pixel = color.argb; // Set pixel color
}

void gfx_draw_line(gfx_buffer* buffer, gfx_line line)
{
    if (!buffer || !buffer->buffer)
    {
        return; // Invalid buffer
    }

    int dx = line.end.x - line.start.x;
    int dy = line.end.y - line.start.y;
    int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
    
    float x_inc = (float)dx / steps;
    float y_inc = (float)dy / steps;

    float x = line.start.x;
    float y = line.start.y;

    for (int i = 0; i <= steps; i++)
    {
        gfx_draw_pixel(buffer, (gfx_point){(int)x, (int)y}, line.color);
        x += x_inc;
        y += y_inc;
    }
}

void gfx_draw_rect(gfx_buffer* buffer, gfx_rect rect, gfx_color color)
{
    if (!buffer)
    {
        return;
    }

    // Top and bottom lines
    for (int x = rect.position.x; x < rect.position.x + rect.size.width; x++)
    {
        gfx_draw_pixel(buffer, (gfx_point){x, rect.position.y}, color);
        gfx_draw_pixel(buffer, (gfx_point){x, rect.position.y + rect.size.height - 1}, color);
    }

    // Left and right lines
    for (int y = rect.position.y; y < rect.position.y + rect.size.height; y++)
    {
        gfx_draw_pixel(buffer, (gfx_point){rect.position.x, y}, color);
        gfx_draw_pixel(buffer, (gfx_point){rect.position.x + rect.size.width - 1, y}, color);
    }
}

void gfx_fill_rect(gfx_buffer* buffer, gfx_rect rect, gfx_color color)
{
    if (!buffer)
    {
        return;
    }

    for (int y = rect.position.y; y < rect.position.y + rect.size.height; y++)
    {
        for (int x = rect.position.x; x < rect.position.x + rect.size.width; x++)
        {
            gfx_draw_pixel(buffer, (gfx_point){x, y}, color);
        }
    }
}

void gfx_draw_char(gfx_buffer* buffer, gfx_point position, char c, gfx_color color)
{
    if (!buffer || !default_font)
    {
        return; // Invalid buffer or no default font loaded
    }

    // Sadece bitmap font destekleniyor şu anda
    if (default_font->type != GFX_FONT_BITMAP)
    {
        return;
    }

    // Karakter aralığını kontrol et (ASCII 32-126 arası)
    if (c < 32 || c > 126)
    {
        return; // Character not in font range
    }

    // Karakter indeksini hesapla
    int char_index = c - 32; // ASCII 32'den başlıyor
    
    // Font boyutlarını al
    int char_width = default_font->size.width;
    int char_height = default_font->size.height;
    
    // Bitmap font verilerini al - her karakter için glyph data
    uint32_t* char_glyph = &default_font->glyphs[char_index * char_height];

    // Her piksel için bitmap'i çiz
    for (int y = 0; y < char_height; y++)
    {
        uint32_t row_data = char_glyph[y];
        for (int x = 0; x < char_width; x++)
        {
            // Eğer piksel aktifse çiz
            if (row_data & (1 << (char_width - 1 - x)))
            {
                gfx_point pixel_pos = {position.x + x, position.y + y};
                gfx_draw_pixel(buffer, pixel_pos, color);
            }
        }
    }
}

void gfx_draw_string(gfx_buffer* buffer, const char* str, gfx_point position, gfx_color color)
{
    if (!buffer || !str || !default_font)
    {
        return;
    }

    gfx_point current_pos = position;
    
    while (*str)
    {
        if (*str == '\n')
        {
            // Yeni satıra geç
            current_pos.x = position.x;
            current_pos.y += default_font->size.height;
        }
        else
        {
            // Karakteri çiz
            gfx_draw_char(buffer, current_pos, *str, color);
            current_pos.x += default_font->size.width;
        }
        str++;
    }
}

void gfx_clear_buffer(gfx_buffer* buffer, gfx_color color)
{
    if (!buffer)
    {
        return;
    }

    gfx_rect full_screen = {{buffer->size.width, buffer->size.height}, {0, 0}, color};
    gfx_fill_rect(buffer, full_screen, color);
}

void gfx_set_font(gfx_font* font)
{
    default_font = font;
}

gfx_font* gfx_get_font()
{
    return default_font;
}


