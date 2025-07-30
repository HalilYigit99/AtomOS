#include <stream/OutputStream.h>
#include <keyboard/Keyboard.h>
#include <graphics/gfx.h>

int main(int argc, char** argv)
{

    gfx_draw_text(screen_buffer, (gfx_point){10, 10}, "Hello, AtomOS!", (gfx_color){.argb = 0xFFFFFFFF});
    gfx_draw_text(screen_buffer, (gfx_point){10, 30}, "Welcome to the graphics demo.", (gfx_color){.argb = 0xFF00FF00});
    gfx_draw_text(screen_buffer, (gfx_point){10, 50}, "Press 'q' to exit.", (gfx_color){.argb = 0xFFFF0000});

    while (1) {
        // Wait for a key press
        while (keyboardInputStream.available() == 0) {
            // Busy wait
        }

        char c;

        if (keyboardInputStream.readChar(&c) >= 0) {
            if (c == 'q') {
                break; // Exit on 'q' key press
            }
        }

    }

    gfx_draw_text(screen_buffer, (gfx_point){10, 70}, "Exiting graphics demo...", (gfx_color){.argb = 0xFF0000FF});

    return 0;

}
