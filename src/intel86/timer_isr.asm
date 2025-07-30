section .text

global timer_isr
extern timer_handle

use32

timer_isr:
    pushad                  ; Save all registers
    call timer_handle      ; Call the timer handler function

    mov al, 0x20        ; Send End of Interrupt (EOI) to PIC
    out 0x20, al  ; Master PIC EOI

    popad
    iret                  ; Return from interrupt