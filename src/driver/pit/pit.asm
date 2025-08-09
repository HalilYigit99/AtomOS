section .text

global pit_isr_handler
extern pit_interrupt_handler

use32

pit_isr_handler:
    cli ; Disable interrupts
    pushad
    
    call pit_interrupt_handler

    mov al, 0x20
    out 0xA0, al    ; Slave PIC EOI
    out 0x20, al    ; Master PIC EOI

    popad
    sti ; Re-enable interrupts
    iret