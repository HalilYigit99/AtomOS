section .multiboot
align 8
multiboot2_header:
    dd 0xe85250d6              ; Multiboot2 magic number
    dd 0                       ; Architecture (0 = i386)
    dd multiboot2_header_end - multiboot2_header  ; Header length
    dd -(0xe85250d6 + 0 + (multiboot2_header_end - multiboot2_header))  ; Checksum

    ; Video mode tag - 1920x1080x32
    align 8
.video_tag_1920x1080:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1920     ; Width
    dd 1080     ; Height
    dd 32       ; Depth

    ; Video mode tag - 1600x900x32
    align 8
.video_tag_1600x900:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1600     ; Width
    dd 900      ; Height
    dd 32       ; Depth

    ; Video mode tag - 1366x768x32
    align 8
.video_tag_1366x768:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1366     ; Width
    dd 768      ; Height
    dd 32       ; Depth

    ; Video mode tag - 1024x768x32 (fallback)
    align 8
.video_tag_1024x768:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1024     ; Width
    dd 768      ; Height
    dd 32       ; Depth

    ; Video mode tag - 800x600x32 (fallback)
    align 8
.video_tag_800x600:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 800      ; Width
    dd 600      ; Height
    dd 32       ; Depth

    ; Text mode tag (if graphics modes fail)
    align 8
.text_mode_tag:
    dw 4        ; Type (console tag)
    dw 1        ; Flags (optional)
    dd 8        ; Size

    align 8
.end_tag:
    ; End tag
    dw 0    ; Type
    dw 0    ; Flags
    dd 8    ; Size
multiboot2_header_end: