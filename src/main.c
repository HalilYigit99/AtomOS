#include <stream/OutputStream.h>
#include <keyboard/Keyboard.h>
#include <graphics/gfx.h>
#include <terminal/terminal.h>
#include <boot/multiboot2.h>
#include <pci/pci.h>

int main(int argc, char** argv)
{
    currentOutputStream->printf("Welcome to AtomOS Terminal!\n");

    VDrawString(screen_buffer, (gfx_point){10, 10}, "Hello, AtomOS!", (gfx_color){.argb = 0xFFFFFFFF}, &gfx_font8x16);

    return 0;
}
