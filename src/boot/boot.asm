section .text

global _start

global mb2_tagptr
extern intel86_setup

extern __stack_start
extern __stack_end

use32
_start:


    ; Save the Multiboot2 tag pointer

    mov [mb2_tagptr], ebx

    ; Set up the stack pointer
    mov esp, __stack_end

    ; Initialize the Intel 86 architecture

    call intel86_setup

.halt:
    hlt
    jmp .halt  ; Infinite loop to halt the CPU


section .data
mb2_tagptr dd 0          ; Pointer to Multiboot2 tag