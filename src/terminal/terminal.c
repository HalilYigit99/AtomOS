#include <terminal/terminal.h>
#include <boot/multiboot2.h>

OutputStream* terminalOutput = NULL;

extern OutputStream terminal_argb_stream;

void terminal_init() {

    currentOutputStream->printf("Initializing terminal...\n");

    if (!mb2_framebuffer) {
        currentOutputStream->printf("No framebuffer found.\n");
        return;
    }

    uint8_t bpp = mb2_framebuffer->framebuffer_bpp;

    // ARGB Terminal
    if (bpp == 32 ) {
        terminalOutput = &terminal_argb_stream;
    }else
    // RGB Terminal
    if (bpp == 24) {

    }else
    // Text Mode Terminal
    if (bpp == 16) {

    }else
    // 256 Color Terminal
    if (bpp == 8) {

    }else {
        currentOutputStream->printf("Unsupported framebuffer format: %d bpp\n", bpp);
        return;
    }

    currentOutputStream->printf("Terminal initialized with %d bpp framebuffer.\n", bpp);

}

