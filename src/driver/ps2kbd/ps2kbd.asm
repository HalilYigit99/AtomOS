section .text

global ps2kbd_isr

extern ps2kbd_handler

use32

ps2kbd_isr:
    cli
    pushad

    call ps2kbd_handler

    mov al, 0x20
    out 0xA0, al    ; Slave PIC EOI
    out 0x20, al    ; Master PIC EOI


    popad
    sti
    iret