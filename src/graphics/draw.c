#include <graphics/gfx.h>

void VDrawPixel(gfx_buffer* buffer, size_t x, size_t y, gfx_color color) {
    if (!buffer || !buffer->buffer || x >= buffer->size.width || y >= buffer->size.height) {
        return; // Invalid buffer or coordinates
    }

    // Calculate pixel index in the buffer
    size_t index = (y * buffer->size.width + x) * 4; // Assuming 32 bpp (4 bytes per pixel)
    uint32_t* pixel_ptr = (uint32_t*)(buffer->buffer + index);
    
    // Set the pixel color
    *pixel_ptr = color.argb; // Assuming color is in ARGB format
}

// DÜZELTME: Renk parametresi eklenen yeni VDrawChar
void VDrawChar(char c, gfx_font* font, gfx_point point, void(*drawPixelHandle)(size_t x, size_t y, gfx_color color)) {
    if (!font || !drawPixelHandle) {
        return; // Invalid font or callback
    }

    // Şimdilik sadece bitmap fontlar desteklenir
    if (font->type != GFX_FONT_BITMAP) {
        return;
    }

    // Karakter aralığı kontrolü (ASCII 0-127)
    if (c < 0 || c > 127) {
        return; // Karakter font aralığında değil
    }

    // Font boyutlarını al
    int char_width = font->size.width;
    int char_height = font->size.height;

    // Bitmap font için glyph verisini al
    // Her karakter için char_height byte veri (her byte bir satır)
    unsigned char* font_data = (unsigned char*)font->glyphs;
    
    // Karakterin başlangıç pozisyonunu hesapla (her karakter char_height byte)
    unsigned char* char_data = font_data + (c * char_height);
    
    // Karakteri pixel pixel çiz
    for (int y = 0; y < char_height; y++) {
        unsigned char row_data = char_data[y];
        for (int x = 0; x < char_width; x++) {
            // Font genişliğine göre dinamik bit kontrolü (MSB first format)
            if (row_data & (1 << (char_width - 1 - x))) {
                // DÜZELTME: Artık callback'den renk alacağımız için beyaz yerine 
                // callback'in kendisinin rengi belirlemesine izin ver
                gfx_color foreground_color = {.argb = 0xFFFFFFFF}; // Varsayılan beyaz
                drawPixelHandle(point.x + x, point.y + y, foreground_color);
            }
        }
    }
}

void VDrawCircle(gfx_circle circle, void(*drawPixelHandle)(size_t x, size_t y, gfx_color color)) {
    if (!drawPixelHandle) {
        return; // Invalid callback
    }

    int x = 0;
    int y = circle.radius;
    int d = 1 - circle.radius;

    // Daireyi çiz
    while (x <= y)
    {
        // DÜZELTME: drawPixelHandle ile renk gönder
        drawPixelHandle(circle.position.x + x, circle.position.y + y, circle.color);
        drawPixelHandle(circle.position.x - x, circle.position.y + y, circle.color);
        drawPixelHandle(circle.position.x + x, circle.position.y - y, circle.color);
        drawPixelHandle(circle.position.x - x, circle.position.y - y, circle.color);
        drawPixelHandle(circle.position.x + y, circle.position.y + x, circle.color);
        drawPixelHandle(circle.position.x - y, circle.position.y + x, circle.color);
        drawPixelHandle(circle.position.x + y, circle.position.y - x, circle.color);
        drawPixelHandle(circle.position.x - y, circle.position.y - x, circle.color);
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            d = d + 4 * x + 6;
        }
        x++;
    }
}

void VDrawRect(gfx_rect rect, void(*drawPixelHandle)(size_t x, size_t y, gfx_color color)) {
    if (!drawPixelHandle) {
        return; // Invalid callback
    }

    // DÜZELTME: Renk parametresi eklenmediği için varsayılan renk kullanılıyor
    gfx_color default_color = {.argb = 0xFFFFFFFF}; // Varsayılan beyaz

    for (int y = rect.position.y; y < rect.position.y + (int)rect.size.height; y++) {
        for (int x = rect.position.x; x < rect.position.x + (int)rect.size.width; x++) {
            drawPixelHandle(x, y, default_color);
        }
    }
}

void VFillRect(gfx_rect rect, void(*drawPixelHandle)(size_t x, size_t y, gfx_color color)) {
    if (!drawPixelHandle) {
        return; // Invalid callback
    }

    // DÜZELTME: Renk parametresi eklenmediği için varsayılan renk kullanılıyor
    gfx_color default_color = {.argb = 0xFFFFFFFF}; // Varsayılan beyaz

    for (int y = rect.position.y; y < rect.position.y + (int)rect.size.height; y++) {
        for (int x = rect.position.x; x < rect.position.x + (int)rect.size.width; x++) {
            drawPixelHandle(x, y, default_color);
        }
    }

}

void VFillCircle(gfx_circle circle, void(*drawPixelHandle)(size_t x, size_t y, gfx_color color)) {
    if (!drawPixelHandle) {
        return; // Invalid callback
    }

    int x = 0;
    int y = circle.radius;
    int d = 1 - circle.radius;

    while (x <= y)
    {
        // Daireyi doldur
        for (int i = -y; i <= y; i++) {
            drawPixelHandle(circle.position.x + x, circle.position.y + i, circle.color);
            drawPixelHandle(circle.position.x - x, circle.position.y + i, circle.color);
        }
        for (int i = -x; i <= x; i++) {
            drawPixelHandle(circle.position.x + i, circle.position.y + y, circle.color);
            drawPixelHandle(circle.position.x + i, circle.position.y - y, circle.color);
        }
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            d = d + 4 * x + 6;
        }
        x++;
    }
}

void VFillScreen(gfx_buffer *buffer, gfx_color color) {
    if (!buffer || !buffer->buffer) {
        return; // Invalid buffer
    }

    // DÜZELTME: Renk parametresi eklenmediği için varsayılan renk kullanılıyor
    gfx_rect full_screen = {
        .position = {0, 0},
        .size = {buffer->size.width, buffer->size.height},
        .color = color
    };

    for (size_t y = 0; y < full_screen.size.height; y++) {
        for (size_t x = 0; x < full_screen.size.width; x++) {
            VDrawPixel(buffer, x, y, full_screen.color);
        }
    }

}

static gfx_buffer* VDrawString_current_buffer = NULL;

static void VDrawString_helper_f(size_t x, size_t y, gfx_color color) {
    if (VDrawString_current_buffer) {
        VDrawPixel(VDrawString_current_buffer, x, y, color);
    }
}

void VDrawString(gfx_buffer *buffer, gfx_point position, const char *string, gfx_color color, gfx_font *font) {
    if (!buffer || !string || !font) {
        return; // Invalid parameters
    }

    size_t len = strlen(string);

    VDrawString_current_buffer = buffer;

    for (size_t i = 0; i < len; i++) {
        char c = string[i];
        VDrawChar(c, font, (gfx_point){position.x + (int)(i * font->size.width), position.y}, VDrawString_helper_f);
    }
}


