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
        uint8_t b; // Blue
        uint8_t g; // Green
        uint8_t r; // Red
        uint8_t a; // Alpha
    };
} gfx_color;

typedef struct {
    gfx_size size;
    gfx_point position;
    gfx_color color;
} gfx_rect;

typedef struct {
    size_t radius;
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
    uint32_t drawBeginLineIndex; // Index of the first line to draw
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

gfx_buffer* gfx_create_buffer(gfx_size size);
void gfx_delete_buffer(gfx_buffer* buffer);

// Çizme yardımcısı fonksiyonları
void VDrawPixel(gfx_buffer* buffer, size_t x, size_t y, gfx_color color);
void VDrawChar(char c, gfx_font* font, gfx_point point, void(*drawPixelHandle)(size_t x, size_t y, gfx_color color));
void VDrawCircle(gfx_circle circle, void(*drawPixelHandle)(size_t x, size_t y, gfx_color color));
void VDrawRect(gfx_rect rect, void(*drawPixelHandle)(size_t x, size_t y, gfx_color color));
void VFillRect(gfx_rect rect, void(*drawPixelHandle)(size_t x, size_t y, gfx_color color));
void VFillCircle(gfx_circle circle, void(*drawPixelHandle)(size_t x, size_t y, gfx_color color));
void VFillScreen(gfx_buffer *buffer, gfx_color color);
void VDrawString(gfx_buffer *buffer, gfx_point position, const char *string, gfx_color color, gfx_font *font);

#ifdef __cplusplus
}
#endif