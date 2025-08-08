section .text

global ps2kbd_isr

extern ps2kbd_handler

use32

ps2kbd_isr:
    cli
    pushad

    call ps2kbd_handler

    mov al, 0x20
    out 0xa0, al    ; slave PIC (IRQ12 buradan gelir)
    out 0x20, al    ; master PIC (slave'den geleni devre dışı bırakmak için)

    popad
    sti
    iret