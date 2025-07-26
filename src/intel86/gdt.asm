section .data

global intel86_gdtr
global intel86_gdt


intel86_gdt:
    ; Null descriptor (required)
    dq 0x0000000000000000
    
    ; Kernel Code Segment (0x08)
    ; Base: 0x00000000, Limit: 0xFFFFF, Access: 0x9A, Flags: 0xCF
    dq 0x00CF9A000000FFFF
    
    ; Kernel Data Segment (0x10)
    ; Base: 0x00000000, Limit: 0xFFFFF, Access: 0x92, Flags: 0xCF
    dq 0x00CF92000000FFFF
    
    ; User Code Segment (0x18)
    ; Base: 0x00000000, Limit: 0xFFFFF, Access: 0xFA, Flags: 0xCF
    dq 0x00CFFA000000FFFF
    
    ; User Data Segment (0x20)
    ; Base: 0x00000000, Limit: 0xFFFFF, Access: 0xF2, Flags: 0xCF
    dq 0x00CFF2000000FFFF
intel86_gdt_end:

intel86_gdtr:
    dw intel86_gdt_end - intel86_gdt - 1  ; Size of GDT (limit)
    dd intel86_gdt                        ; Address of GDT
