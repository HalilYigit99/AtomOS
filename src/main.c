#include <stream/OutputStream.h>
#include <keyboard/Keyboard.h>
#include <graphics/gfx.h>
#include <terminal/terminal.h>
#include <boot/multiboot2.h>
#include <pci/pci.h>

int __attribute__((optimize("O0"))) main(int argc, char** argv)
{
    currentOutputStream->printf("Welcome to AtomOS Terminal!\n");

    return 0;
}