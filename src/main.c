#include <stream/OutputStream.h>
#include <keyboard/Keyboard.h>
#include <graphics/gfx.h>
#include <terminal/terminal.h>
#include <boot/multiboot2.h>
#include <pci/pci.h>

extern gfx_buffer terminal_argb_buffer;

int __attribute__((optimize("O0"))) main(int argc, char** argv)
{

    currentOutputStream->printf("Welcome to AtomOS Terminal!\n");

    gfx_draw_text(&screen_buffer, (gfx_point){10, 10}, "Direct Framebuffer Text", (gfx_color){.argb = 0xFFFFFFFF}, &gfx_font8x16);

    gfx_draw_line(&screen_buffer, (gfx_line){
        .start = {10, 30},
        .end = {200, 30},
        .color = {.argb = 0xFF00FF00},
        .thickness = 2
    });

    currentOutputStream->printf("Welcome to AtomOS Terminal!\n");

    gfx_draw();

    return 0;

}
