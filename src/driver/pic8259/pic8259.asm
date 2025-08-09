section .text

global irq2_isr
extern irq2_handler

irq2_isr:
    cli

    pushad

    call irq2_handler

    popad

    ; irq2_isr_address'teki adresi stack'e push et ve oraya jump et
    push dword [irq2_isr_address]
    ret

section .data
global irq2_isr_address
irq2_isr_address: dd 0

global irq_default_handler
irq_default_handler:
    cli
    push eax
    mov al, 0x20
    out 0x20, al    ; Master PIC EOI
    out 0xA0, al    ; Slave PIC EOI
    pop eax
    sti
    iret
