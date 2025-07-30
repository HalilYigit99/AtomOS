; PIC (Programmable Interrupt Controller) setup for Intel86
; Master PIC: 0x20 (command), 0x21 (data)
; Slave PIC:  0xA0 (command), 0xA1 (data)

PIC1_COMMAND equ 0x20
PIC1_DATA   equ 0x21
PIC2_COMMAND equ 0xA0
PIC2_DATA   equ 0xA1

PIC_EOI     equ 0x20

section .text
global intel86_pic_init

intel86_pic_init:
    ; Reset PICs (ICW1)
    mov al, 0x11
    out PIC1_COMMAND, al
    out PIC2_COMMAND, al

    ; Remap PICs (ICW2)
    mov al, 0x20        ; Master PIC vector offset (0x20)
    out PIC1_DATA, al
    mov al, 0x28        ; Slave PIC vector offset (0x28)
    out PIC2_DATA, al

    ; Setup cascading (ICW3)
    mov al, 0x04        ; Master PIC: Slave on IRQ2
    out PIC1_DATA, al
    mov al, 0x02        ; Slave PIC: Cascade identity
    out PIC2_DATA, al

    ; Set environment info (ICW4)
    mov al, 0x01        ; 8086/88 mode
    out PIC1_DATA, al
    out PIC2_DATA, al

    ; Mask all IRQs (IMR)
    mov al, 0xFF
    out PIC1_DATA, al
    out PIC2_DATA, al

    ret


