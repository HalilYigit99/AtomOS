section .text
use32

global ps2mouse_isr
extern ps2mouse_isr_handler

ps2mouse_isr:
    cli
    ; Save registers that will be used
    pushad

    ; Call the handler function
    call ps2mouse_isr_handler

    ; Send End of Interrupt (EOI) signal to the PIC
    mov al, 0x20
    out 0xa0, al

    ; Restore registers
    popad

    sti
    ; Return from interrupt
    iret
