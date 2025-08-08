#include <stream/OutputStream.h>
#include <keyboard/Keyboard.h>
#include <terminal/terminal.h>
#include <boot/multiboot2.h>
#include <pci/pci.h>
#include <graphics/gfx.h>
#include <task/PeriodicTask.h>
#include <util.h>

int main(int argc  __attribute__((unused)), char** argv  __attribute__((unused)))
{
    currentOutputStream->printf("Welcome to AtomOS Terminal!\n");

    char foo;

    uint32_t color = 1;
    while (color) {
        gfx_color gfx = (gfx_color){ .argb = color };
        gfx.a = 255;
        gfx_clear_buffer(screen_buffer, gfx); // Clear screen with gray color
        color++;
        if (keyboardInputStream.available()) {
            int result = keyboardInputStream.readChar(&foo);
            if (foo == 'q') {
                currentOutputStream->printf("Exiting terminal...\n");
                return 0;
            }
        }
    }

    return 0;
}
