section .multiboot
align 8
multiboot2_header:
    dd 0xe85250d6              ; Multiboot2 magic number
    dd 0                       ; Architecture (0 = i386)
    dd multiboot2_header_end - multiboot2_header  ; Header length
    dd -(0xe85250d6 + 0 + (multiboot2_header_end - multiboot2_header))  ; Checksum

    ; High resolution video modes
    
    ; Video mode tag - 3840x2160x32 (4K)
    align 8
.video_tag_3840x2160:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 3840     ; Width
    dd 2160     ; Height
    dd 32       ; Depth

    ; Video mode tag - 2560x1440x32 (1440p)
    align 8
.video_tag_2560x1440:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 2560     ; Width
    dd 1440     ; Height
    dd 32       ; Depth

    ; Video mode tag - 1920x1200x32 (WUXGA)
    align 8
.video_tag_1920x1200:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1920     ; Width
    dd 1200     ; Height
    dd 32       ; Depth

    ; Video mode tag - 1920x1080x32 (Full HD)
    align 8
.video_tag_1920x1080:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1920     ; Width
    dd 1080     ; Height
    dd 32       ; Depth

    ; Video mode tag - 1680x1050x32 (WSXGA+)
    align 8
.video_tag_1680x1050:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1680     ; Width
    dd 1050     ; Height
    dd 32       ; Depth

    ; Video mode tag - 1600x1200x32 (UXGA)
    align 8
.video_tag_1600x1200:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1600     ; Width
    dd 1200     ; Height
    dd 32       ; Depth

    ; Video mode tag - 1600x900x32 (HD+)
    align 8
.video_tag_1600x900:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1600     ; Width
    dd 900      ; Height
    dd 32       ; Depth

    ; Video mode tag - 1536x864x32 (Common laptop resolution)
    align 8
.video_tag_1536x864:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1536     ; Width
    dd 864      ; Height
    dd 32       ; Depth

    ; Video mode tag - 1440x900x32 (WXGA+)
    align 8
.video_tag_1440x900:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1440     ; Width
    dd 900      ; Height
    dd 32       ; Depth

    ; Video mode tag - 1400x1050x32 (SXGA+)
    align 8
.video_tag_1400x1050:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1400     ; Width
    dd 1050     ; Height
    dd 32       ; Depth

    ; Video mode tag - 1366x768x32 (HD, very common on laptops)
    align 8
.video_tag_1366x768:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1366     ; Width
    dd 768      ; Height
    dd 32       ; Depth

    ; Video mode tag - 1360x768x32 (Alternative HD)
    align 8
.video_tag_1360x768:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1360     ; Width
    dd 768      ; Height
    dd 32       ; Depth

    ; Video mode tag - 1280x1024x32 (SXGA)
    align 8
.video_tag_1280x1024:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1280     ; Width
    dd 1024     ; Height
    dd 32       ; Depth

    ; Video mode tag - 1280x960x32 (4:3 ratio)
    align 8
.video_tag_1280x960:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1280     ; Width
    dd 960      ; Height
    dd 32       ; Depth

    ; Video mode tag - 1280x800x32 (WXGA, common on laptops)
    align 8
.video_tag_1280x800:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1280     ; Width
    dd 800      ; Height
    dd 32       ; Depth

    ; Video mode tag - 1280x720x32 (HD 720p)
    align 8
.video_tag_1280x720:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1280     ; Width
    dd 720      ; Height
    dd 32       ; Depth

    ; Video mode tag - 1152x864x32 (XGA+)
    align 8
.video_tag_1152x864:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1152     ; Width
    dd 864      ; Height
    dd 32       ; Depth

    ; Video mode tag - 1024x768x32 (XGA, widely supported)
    align 8
.video_tag_1024x768:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1024     ; Width
    dd 768      ; Height
    dd 32       ; Depth

    ; Video mode tag - 1024x600x32 (WSVGA, netbook resolution)
    align 8
.video_tag_1024x600:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 1024     ; Width
    dd 600      ; Height
    dd 32       ; Depth

    ; Video mode tag - 800x600x32 (SVGA)
    align 8
.video_tag_800x600:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 800      ; Width
    dd 600      ; Height
    dd 32       ; Depth

    ; Video mode tag - 640x480x32 (VGA, almost universally supported)
    align 8
.video_tag_640x480:
    dw 5        ; Type (framebuffer tag)
    dw 0        ; Flags
    dd 20       ; Size
    dd 640      ; Width
    dd 480      ; Height
    dd 32       ; Depth

    align 8
.end_tag:
    ; End tag
    dw 0    ; Type
    dw 0    ; Flags
    dd 8    ; Size
multiboot2_header_end: