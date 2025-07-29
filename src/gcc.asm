; gcc.asm - GCC compiler helper functions for 64-bit arithmetic on 32-bit systems
; These functions are required by GCC for 64-bit division operations on i386
; Based on LLVM compiler-rt and GLIBC libgcc implementations

BITS 32

section .text

; Unsigned 64-bit division with remainder
; Input: EDX:EAX = dividend, ECX:EBX = divisor
; Output: EDX:EAX = quotient, ECX:EBX = remainder
global __udivmoddi4
__udivmoddi4:
    push ebp
    mov ebp, esp
    push edi
    push esi
    push ebx

    ; Load arguments from stack
    mov eax, [ebp+8]    ; dividend low
    mov edx, [ebp+12]   ; dividend high
    mov ebx, [ebp+16]   ; divisor low
    mov ecx, [ebp+20]   ; divisor high

    ; Test for 32-bit case
    test ecx, ecx
    jnz .L4
    cmp edx, ebx
    jae .L3
    
    ; Case: dividend < divisor (32-bit)
    div ebx
    mov ecx, edx        ; remainder
    xor edx, edx        ; high quotient = 0
    xor ebx, ebx        ; high remainder = 0
    jmp .L6

.L3:
    ; Case: dividend high >= divisor (32-bit)
    mov ecx, eax        ; save dividend low
    mov eax, edx        ; dividend high
    xor edx, edx
    div ebx             ; divide high part
    xchg eax, ecx       ; quotient high -> ecx, dividend low -> eax
    div ebx             ; divide low part
    mov edx, ecx        ; quotient high
    mov ecx, eax        ; quotient low
    mov eax, ecx        ; restore eax
    mov ecx, edx        ; remainder in ecx
    xor ebx, ebx        ; high remainder = 0
    jmp .L6

.L4:
    ; Full 64-bit division
    cmp ecx, edx
    ja .L5
    jb .L8
    cmp ebx, eax
    ja .L5

.L8:
    ; Use binary long division algorithm
    push edi
    push esi
    
    ; Initialize
    xor edi, edi        ; quotient high
    xor esi, esi        ; quotient low
    mov ebp, 64         ; bit counter

.L9:
    ; Shift dividend left
    shl eax, 1
    rcl edx, 1
    
    ; Shift quotient left
    shl esi, 1
    rcl edi, 1
    
    ; Compare with divisor
    cmp edx, ecx
    jb .L10
    ja .L11
    cmp eax, ebx
    jb .L10

.L11:
    ; Subtract divisor
    sub eax, ebx
    sbb edx, ecx
    inc esi             ; set quotient bit

.L10:
    dec ebp
    jnz .L9
    
    ; Move results
    mov ecx, edx        ; remainder high
    mov ebx, eax        ; remainder low
    mov eax, esi        ; quotient low
    mov edx, edi        ; quotient high
    
    pop esi
    pop edi
    jmp .L6

.L5:
    ; Quotient is zero
    mov ecx, edx        ; remainder = dividend
    mov ebx, eax
    xor eax, eax        ; quotient = 0
    xor edx, edx

.L6:
    pop ebx
    pop esi
    pop edi
    pop ebp
    ret

; Unsigned 64-bit division
; Returns quotient only
global __udivdi3
__udivdi3:
    call __udivmoddi4
    ret

; Unsigned 64-bit modulo
; Returns remainder only
global __umoddi3
__umoddi3:
    call __udivmoddi4
    mov eax, ebx
    mov edx, ecx
    ret

; Signed 64-bit division with remainder
global __divmoddi4
__divmoddi4:
    push ebp
    mov ebp, esp
    push edi
    push esi
    push ebx

    ; Load arguments
    mov eax, [ebp+8]    ; dividend low
    mov edx, [ebp+12]   ; dividend high
    mov ebx, [ebp+16]   ; divisor low
    mov ecx, [ebp+20]   ; divisor high

    ; Determine result signs
    xor edi, edi        ; quotient sign
    xor esi, esi        ; remainder sign

    ; Check dividend sign
    test edx, edx
    jns .LD1
    ; Negate dividend
    neg eax
    adc edx, 0
    neg edx
    inc edi             ; quotient negative
    inc esi             ; remainder has dividend sign

.LD1:
    ; Check divisor sign
    test ecx, ecx
    jns .LD2
    ; Negate divisor
    neg ebx
    adc ecx, 0
    neg ecx
    inc edi             ; toggle quotient sign

.LD2:
    ; Perform unsigned division
    push edi
    push esi
    call __udivmoddi4
    pop esi
    pop edi

    ; Apply quotient sign
    test edi, edi
    jz .LD3
    neg eax
    adc edx, 0
    neg edx

.LD3:
    ; Apply remainder sign
    test esi, esi
    jz .LD4
    neg ebx
    adc ecx, 0
    neg ecx

.LD4:
    pop ebx
    pop esi
    pop edi
    pop ebp
    ret

; Signed 64-bit division
global __divdi3
__divdi3:
    call __divmoddi4
    ret

; Signed 64-bit modulo
global __moddi3
__moddi3:
    call __divmoddi4
    mov eax, ebx
    mov edx, ecx
    ret

; 64-bit multiplication
; Stack: [multiplier_low][multiplier_high][multiplicand_low][multiplicand_high][ret_addr]
global __muldi3
__muldi3:
    push ebp
    mov ebp, esp
    push ebx
    
    ; Load arguments
    mov eax, [ebp+8]    ; multiplicand low
    mov ecx, [ebp+12]   ; multiplicand high
    mov ebx, [ebp+16]   ; multiplier low
    mov edx, [ebp+20]   ; multiplier high
    
    ; Multiply: (a_low + a_high*2^32) * (b_low + b_high*2^32)
    ; = a_low*b_low + (a_low*b_high + a_high*b_low)*2^32 + a_high*b_high*2^64
    
    push eax
    push edx
    
    ; Calculate a_low * b_low
    mul ebx             ; EDX:EAX = a_low * b_low
    push edx            ; Save high part
    push eax            ; Save low part
    
    ; Calculate a_low * b_high
    mov eax, [ebp+8]
    mul dword [ebp+20]
    mov ebx, eax        ; Save result
    
    ; Calculate a_high * b_low
    mov eax, ecx
    mul dword [ebp+16]
    add ebx, eax        ; Add to previous result
    
    ; Combine results
    pop eax             ; Low part of result
    pop edx             ; High part of a_low * b_low
    add edx, ebx        ; Add cross products
    
    pop ebx
    pop ebx
    
    pop ebx
    pop ebp
    ret

; Arithmetic shift right
; Stack: [shift_count][value_low][value_high][ret_addr]
global __ashrdi3
__ashrdi3:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp+8]    ; value low
    mov edx, [ebp+12]   ; value high
    mov ecx, [ebp+16]   ; shift count
    
    and ecx, 63         ; Limit shift count
    
    cmp ecx, 32
    jae .shift_32_or_more
    
    ; Shift less than 32
    shrd eax, edx, cl
    sar edx, cl
    jmp .done
    
.shift_32_or_more:
    ; Shift 32 or more
    sub ecx, 32
    mov eax, edx
    sar eax, cl
    sar edx, 31         ; Sign extend
    
.done:
    pop ebp
    ret

; Logical shift right
global __lshrdi3
__lshrdi3:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp+8]    ; value low
    mov edx, [ebp+12]   ; value high
    mov ecx, [ebp+16]   ; shift count
    
    and ecx, 63         ; Limit shift count
    
    cmp ecx, 32
    jae .shift_32_or_more
    
    ; Shift less than 32
    shrd eax, edx, cl
    shr edx, cl
    jmp .done
    
.shift_32_or_more:
    ; Shift 32 or more
    sub ecx, 32
    mov eax, edx
    shr eax, cl
    xor edx, edx        ; Zero extend
    
.done:
    pop ebp
    ret

; Logical shift left
global __ashldi3
__ashldi3:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp+8]    ; value low
    mov edx, [ebp+12]   ; value high
    mov ecx, [ebp+16]   ; shift count
    
    and ecx, 63         ; Limit shift count
    
    cmp ecx, 32
    jae .shift_32_or_more
    
    ; Shift less than 32
    shld edx, eax, cl
    shl eax, cl
    jmp .done
    
.shift_32_or_more:
    ; Shift 32 or more
    sub ecx, 32
    mov edx, eax
    shl edx, cl
    xor eax, eax
    
.done:
    pop ebp
    ret