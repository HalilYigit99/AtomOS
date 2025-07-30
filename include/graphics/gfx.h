#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uint32_t width;
    uint32_t height;
} gfx_size;

typedef struct {
    int x;
    int y;
} gfx_point;

typedef union {
    uint32_t argb;
    struct {
        uint8_t a; // Alpha
        uint8_t r; // Red
        uint8_t g; // Green
        uint8_t b; // Blue
    };
} gfx_color;

typedef struct {
    gfx_size size;
    gfx_point position;
    gfx_color color;
} gfx_rect;

typedef struct {
    gfx_size radius;
    gfx_point position;
    gfx_color color;
} gfx_circle;

typedef struct {
    gfx_point start;
    gfx_point end;
    gfx_color color;
    size_t thickness; // Thickness of the line
} gfx_line;

typedef struct {
    gfx_size size;
    void* buffer; // Pointer to the pixel buffer
    uint32_t bpp; // Bits per pixel
} gfx_buffer;

typedef enum {
    GFX_FONT_BITMAP, // Bitmap font
    GFX_FONT_VECTOR, // Vector font
    GFX_FONT_PSF,    // PostScript font
    GFX_FONT_TTF,    // TrueType font
    GFX_FONT_OTF     // OpenType font
} gfx_font_type;

typedef struct {
    char* name; // Font name
    gfx_size size; // Font size
    uint32_t* glyphs; // Pointer to glyph data (bitmap or vector)
    gfx_font_type type; // Type of the font
} gfx_font;

extern gfx_buffer* screen_buffer; // Global screen buffer
extern gfx_font* default_font; // Global default font

extern gfx_font gfx_font8x8; // 8x8 bitmap font
extern gfx_font gfx_font8x16; // 8x16 bitmap font

void gfx_copy(gfx_buffer* src, gfx_buffer* dest, gfx_point src_pos, gfx_point dest_pos, gfx_size size);

void gfx_draw_pixel(gfx_buffer* buffer, gfx_point position, gfx_color color);
void gfx_fill_rect(gfx_buffer* buffer, gfx_rect rect, gfx_color color);
void gfx_draw_circle(gfx_buffer* buffer, gfx_circle circle);
void gfx_draw_line(gfx_buffer* buffer, gfx_line line);
void gfx_draw_char(gfx_buffer* buffer, gfx_point position, char character, gfx_color color);
void gfx_draw_text(gfx_buffer* buffer, gfx_point position, const char* text, gfx_color color);
void gfx_clear(gfx_buffer* buffer, gfx_color color);

#ifdef __cplusplus
}
#endif
