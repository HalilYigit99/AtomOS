#include <io.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

void pic_mask(unsigned char id) {

    if (id >= 16) {
        // Invalid ID, do nothing
        return;
    }

    if (id < 8) {
        // Mask PIC1
        uint8_t mask = inb(PIC1_DATA);
        outb(PIC1_DATA, mask | (1 << id));
    } else {
        // Mask PIC2
        id -= 8; // Adjust for PIC2
        uint8_t mask = inb(PIC2_DATA);
        outb(PIC2_DATA, mask | (1 << id));
    }
}

void pic_unmask(unsigned char id) {

    if (id >= 16) {
        // Invalid ID, do nothing
        return;
    }

    if (id < 8) {
        // Unmask PIC1
        uint8_t mask = inb(PIC1_DATA);
        outb(PIC1_DATA, mask & ~(1 << id));
    } else {
        // Unmask PIC2
        id -= 8; // Adjust for PIC2
        uint8_t mask = inb(PIC2_DATA);
        outb(PIC2_DATA, mask & ~(1 << id));
    }
}
