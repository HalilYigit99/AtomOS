section .text

global pit_isr_handler
extern pit_interrupt_handler

use32

pit_isr_handler:
    cli ; Disable interrupts
    pushad
    
    call pit_interrupt_handler

    mov al, 0x20
    out 0xa0, al    ; slave PIC (IRQ12 buradan gelir)
    out 0x20, al    ; master PIC (slave'den geleni devre dışı bırakmak için)


    popad
    sti ; Re-enable interrupts
    iret