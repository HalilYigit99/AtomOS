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

    gfx_clear_buffer(screen_buffer, (gfx_color){ .argb = 0 }); // Clear screen with gray color

    return 0;
}
