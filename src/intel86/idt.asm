section .bss
use32

global intel86_idt
global intel86_idtr

intel86_idt: resb 256 * 8  ; Reserve space for 256 IDT entries (8 bytes each)

section .data

intel86_idtr:
    dw 256 * 8 - 1          ; Size of IDT (limit)
    dd intel86_idt          ; Address of IDT

section .text

global intel86_isr_default

; Default ISR handler
intel86_isr_default:
    iret