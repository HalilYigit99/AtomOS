section .text

global _start

global mb2_tagptr
extern intel86_setup
extern __kernel_setup
extern main
extern __stack_start
extern __stack_end

use32
_start:
    cli ; Disable interrupts
    ; Save the Multiboot2 tag pointer
    mov [mb2_tagptr], ebx

    cmp eax, 0x36d76289 ; Check if the magic number is correct
    jne .ret ;

    ; Set up the stack pointer
    mov esp, __stack_end

    ; Initialize the Intel 86 architecture
    call intel86_setup

    ; Setup kernel heap and other kernel structures
    call __kernel_setup

    push 0 ; Push argc (0 for now)
    push 0 ; Push argv (NULL for now)
    call main ; Call the main kernel function

.halt:
    hlt
    jmp .halt  ; Infinite loop to halt the CPU

.ret:
    ; If the magic number is incorrect, halt the CPU
    cli
    hlt
    ret


section .data
mb2_tagptr dd 0          ; Pointer to Multiboot2 tag