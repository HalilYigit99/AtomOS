section .multiboot
align 8
multiboot2_header:
    dd 0xe85250d6              ; Multiboot2 magic number
    dd 0                       ; Architecture (0 = i386)
    dd multiboot2_header_end - multiboot2_header  ; Header length
    dd -(0xe85250d6 + 0 + (multiboot2_header_end - multiboot2_header))  ; Checksum

    align 8
    .video_tag_800x600:
        dw 5        ; Type (framebuffer tag)
        dw 0        ; Flags
        dd 20       ; Size
        dd 800      ; Width
        dd 600      ; Height
        dd 32       ; Depth

    align 8
.end_tag:
    ; End tag
    dw 0    ; Type
    dw 0    ; Flags
    dd 8    ; Size
multiboot2_header_end: