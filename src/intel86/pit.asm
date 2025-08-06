section .text

global pit_isr_handler
extern pit_interrupt_handler

use32

pit_isr_handler:
    cli ; Disable interrupts
    pushad
    
    call pit_interrupt_handler

    mov al, 0x20
    out 0x20, al  ; Send End of Interrupt (EOI) to the PIC

    popad
    sti ; Re-enable interrupts
    iret