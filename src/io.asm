; io.asm - IO Port access functions for 32-bit x86
; Provides low-level port IO operations

BITS 32

section .text

; uint8_t inb(uint16_t port)
; Read a byte from the specified port
global inb
inb:
    push ebp
    mov ebp, esp
    
    mov dx, [ebp + 8]   ; Port number
    xor eax, eax        ; Clear eax
    in al, dx           ; Read byte from port
    
    pop ebp
    ret

; uint16_t inw(uint16_t port)
; Read a word (2 bytes) from the specified port
global inw
inw:
    push ebp
    mov ebp, esp
    
    mov dx, [ebp + 8]   ; Port number
    xor eax, eax        ; Clear eax
    in ax, dx           ; Read word from port
    
    pop ebp
    ret

; uint32_t inl(uint16_t port)
; Read a dword (4 bytes) from the specified port
global inl
inl:
    push ebp
    mov ebp, esp
    
    mov dx, [ebp + 8]   ; Port number
    in eax, dx          ; Read dword from port
    
    pop ebp
    ret

; void outb(uint16_t port, uint8_t value)
; Write a byte to the specified port
global outb
outb:
    push ebp
    mov ebp, esp
    
    mov dx, [ebp + 8]   ; Port number
    mov al, [ebp + 12]  ; Value to write
    out dx, al          ; Write byte to port
    
    pop ebp
    ret

; void outw(uint16_t port, uint16_t value)
; Write a word (2 bytes) to the specified port
global outw
outw:
    push ebp
    mov ebp, esp
    
    mov dx, [ebp + 8]   ; Port number
    mov ax, [ebp + 12]  ; Value to write
    out dx, ax          ; Write word to port
    
    pop ebp
    ret

; void outl(uint16_t port, uint32_t value)
; Write a dword (4 bytes) to the specified port
global outl
outl:
    push ebp
    mov ebp, esp
    
    mov dx, [ebp + 8]   ; Port number
    mov eax, [ebp + 12] ; Value to write
    out dx, eax         ; Write dword to port
    
    pop ebp
    ret

; void insb(uint16_t port, void* buffer, uint32_t count)
; Read multiple bytes from port into buffer
global insb
insb:
    push ebp
    mov ebp, esp
    push edi
    
    mov dx, [ebp + 8]   ; Port number
    mov edi, [ebp + 12] ; Buffer address
    mov ecx, [ebp + 16] ; Count
    
    cld                 ; Clear direction flag (forward)
    rep insb            ; Repeat: Read byte from port to [edi], increment edi
    
    pop edi
    pop ebp
    ret

; void insw(uint16_t port, void* buffer, uint32_t count)
; Read multiple words from port into buffer
global insw
insw:
    push ebp
    mov ebp, esp
    push edi
    
    mov dx, [ebp + 8]   ; Port number
    mov edi, [ebp + 12] ; Buffer address
    mov ecx, [ebp + 16] ; Count
    
    cld                 ; Clear direction flag (forward)
    rep insw            ; Repeat: Read word from port to [edi], increment edi by 2
    
    pop edi
    pop ebp
    ret

; void insl(uint16_t port, void* buffer, uint32_t count)
; Read multiple dwords from port into buffer
global insl
insl:
    push ebp
    mov ebp, esp
    push edi
    
    mov dx, [ebp + 8]   ; Port number
    mov edi, [ebp + 12] ; Buffer address
    mov ecx, [ebp + 16] ; Count
    
    cld                 ; Clear direction flag (forward)
    rep insd            ; Repeat: Read dword from port to [edi], increment edi by 4
    
    pop edi
    pop ebp
    ret

; void outsb(uint16_t port, const void* buffer, uint32_t count)
; Write multiple bytes from buffer to port
global outsb
outsb:
    push ebp
    mov ebp, esp
    push esi
    
    mov dx, [ebp + 8]   ; Port number
    mov esi, [ebp + 12] ; Buffer address
    mov ecx, [ebp + 16] ; Count
    
    cld                 ; Clear direction flag (forward)
    rep outsb           ; Repeat: Write byte from [esi] to port, increment esi
    
    pop esi
    pop ebp
    ret

; void outsw(uint16_t port, const void* buffer, uint32_t count)
; Write multiple words from buffer to port
global outsw
outsw:
    push ebp
    mov ebp, esp
    push esi
    
    mov dx, [ebp + 8]   ; Port number
    mov esi, [ebp + 12] ; Buffer address
    mov ecx, [ebp + 16] ; Count
    
    cld                 ; Clear direction flag (forward)
    rep outsw           ; Repeat: Write word from [esi] to port, increment esi by 2
    
    pop esi
    pop ebp
    ret

; void outsl(uint16_t port, const void* buffer, uint32_t count)
; Write multiple dwords from buffer to port
global outsl
outsl:
    push ebp
    mov ebp, esp
    push esi
    
    mov dx, [ebp + 8]   ; Port number
    mov esi, [ebp + 12] ; Buffer address
    mov ecx, [ebp + 16] ; Count
    
    cld                 ; Clear direction flag (forward)
    rep outsd           ; Repeat: Write dword from [esi] to port, increment esi by 4
    
    pop esi
    pop ebp
    ret

; void io_wait(void)
; Perform a small delay by doing a dummy write to port 0x80
; This is commonly used to give slow IO devices time to respond
global io_wait
io_wait:
    push eax
    mov al, 0
    out 0x80, al        ; Dummy write to unused port
    pop eax
    ret