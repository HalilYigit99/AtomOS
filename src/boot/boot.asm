section .text

global _start

global mb2_tagptr
global mb2_signature
extern __kernel_setup
extern __stack_start
extern __stack_end

use32
_start:
    cli ; Disable interrupts

    ; Check for Long Mode is enabled
    mov edx, cr0
    test edx, 0x80000000 ; Check if Long Mode is enabled
    jnz .long_mode_disable ; If Long Mode is enabled, halt

    mov [mb2_signature], eax ; Save Multiboot2 signature
    mov [mb2_tagptr], ebx ; Save Multiboot2 tag pointer

.initKernel:

    cmp eax, 0x36d76289 ; Check if the magic number is correct
    jne .ret ;

    ; Set up the stack pointer
    mov esp, __stack_end

    call __kernel_setup ; Call the kernel setup function

.halt:
    hlt
    jmp .halt  ; Infinite loop to halt the CPU

.ret:
    ; If the magic number is incorrect, halt the CPU
    cli
    hlt
    ret

use64
.long_mode_disable:
    ; Long mode'dan güvenli çıkış
    cli

    ; Save the Multiboot2 tag pointer
    mov [mb2_tagptr], ebx
    mov [mb2_signature], eax

    ; Paging'i kapat
    mov rax, cr0
    and rax, 0x7FFFFFFF  ; CR0.PG bit'ini temizle
    mov cr0, rax

    ; 1.1 PAE varsa kapat
    mov rax, cr4
    and rax, 0xFFFFFFFE  ; CR4.PAE bit'ini temizle
    mov cr4, rax

    ; ÖNCE TLB'yi temizle
    mov rax, cr3
    mov cr3, rax
    
    ; Long Mode page table'larını sıfırla
    xor rax, rax
    mov cr3, rax
    
    ; 2. EFER.LME bit'ini temizle (Long mode'u kapat)
    mov rcx, 0xC0000080   ; EFER MSR
    rdmsr
    and eax, 0xFFFFFEFF   ; LME bit'ini temizle (bit 8)
    wrmsr

use32

    lgdt [temp_gdtr]

    jmp 0x08:.refresh_segments  ; Far jump ile 32-bit mod'a geç
    ; 3. Compatibility mode'a düş (far jump yok, direkt düşer)
    ; Artık 32-bit instruction'lar çalışır
    
.refresh_segments:
    ; 6. Segment register'ları ayarla
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    mov eax, [mb2_signature]
    mov ebx, [mb2_tagptr]

    jmp .initKernel

section .data
mb2_tagptr dd 0          ; Pointer to Multiboot2 tag
mb2_signature dd 0      ; Multiboot2 signature

; Geçici GDT Long Mode'dan çıkmak için
align 8
temp_gdt:
    ; Null descriptor (0x00)
    dq 0x0000000000000000
    
    ; 32-bit Code Segment (0x08)
    ; Base: 0x00000000, Limit: 0xFFFFF, Access: 0x9A, Flags: 0xCF
    dq 0x00CF9A000000FFFF
    
    ; 32-bit Data Segment (0x10) 
    ; Base: 0x00000000, Limit: 0xFFFFF, Access: 0x92, Flags: 0xCF
    dq 0x00CF92000000FFFF
temp_gdt_end:

temp_gdtr:
    dw temp_gdt_end - temp_gdt - 1  ; GDT limit
    dd temp_gdt                     ; GDT base address