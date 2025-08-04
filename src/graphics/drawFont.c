#include <graphics/gfx.h>

/**
 * VDrawChar - Generic character drawing function
 * 
 * Bu fonksiyon, verilen font kullanarak karakteri pixel pixel çizer.
 * Çizim işlemi için verilen drawPixelHandle callback fonksiyonunu kullanır.
 * Bu sayede farklı çizim hedefleri (framebuffer, texture, etc.) desteklenebilir.
 * 
 * @param c Çizilecek karakter
 * @param font Kullanılacak font (şimdilik sadece bitmap fontlar desteklenir)
 * @param point Karakterin çizileceği başlangıç pozisyonu
 * @param drawPixelHandle Pixel çizme callback fonksiyonu
 */
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
            // Bit set ise pixel çiz (MSB'den başlayarak, 7-x ile bit sırasını kontrol et)
            if (row_data & (1 << (7 - x))) {
                gfx_color foreground_color = {.argb = 0xFFFFFFFF}; // Varsayılan beyaz
                drawPixelHandle(point.x + x, point.y + y, foreground_color);
            }
        }
    }
    
    // TODO: İleride farklı font boyutları ve türleri için destek eklenebilir
    // TODO: Renk parametresi eklenerek daha esnek yapılabilir
    // TODO: Karakter aralığı genişletilebilir (Unicode desteği)
}

