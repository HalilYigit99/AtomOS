section .text

global ps2kbd_isr

extern ps2kbd_handler

use32

ps2kbd_isr:
    pushad

    call ps2kbd_handler

    mov ax, 0x20
    out 0x20, al  ; Send End of Interrupt (EOI) to the PIC

    popad
    iret