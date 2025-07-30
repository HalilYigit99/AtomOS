section .text
global intel86_setup

extern intel86_gdtr
extern intel86_idt_init
extern intel86_paging_init
extern intel86_pic_init

use32

intel86_setup:

    ; Load the Global Descriptor Table (GDT)
    lgdt [intel86_gdtr]

    jmp 0x08:intel86_setup_next  ; Jump to code segment with proper privilege level
intel86_setup_next:
    
    ; Set up the data segment registers
    mov ax, 0x10                ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax                  ; Stack segment selector

    call intel86_idt_init  ; Initialize the Interrupt Descriptor Table (IDT)

    call intel86_paging_init  ; Initialize paging

    call intel86_pic_init  ; Initialize the Programmable Interrupt Controller (PIC)

    ; Enable interrupts
    sti

    ret
