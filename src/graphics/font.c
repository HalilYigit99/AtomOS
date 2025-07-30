#include <graphics/gfx.h>

extern unsigned char __font8x8[][8];
extern unsigned char __font8x16[][16];

gfx_font gfx_font8x8 = {
    .name = "8x8 Bitmap Font",
    .size = {8, 8},
    .glyphs = (uint32_t*)__font8x8,
    .type = GFX_FONT_BITMAP
};
gfx_font gfx_font8x16 = {
    .name = "8x16 Bitmap Font",
    .size = {8, 16},
    .glyphs = (uint32_t*)__font8x16,
    .type = GFX_FONT_BITMAP
};
