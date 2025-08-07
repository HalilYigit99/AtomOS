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

    

    return 0;
}
