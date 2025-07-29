; gcc.asm - GCC compiler helper functions for 64-bit arithmetic on 32-bit systems
; These functions are required by GCC for 64-bit division operations on i386

BITS 32

section .text

; Signed 64-bit division with remainder
; Stack: [divisor_low][divisor_high][dividend_low][dividend_high][ret_addr]
; Returns: EDX:EAX = quotient, ECX:EBX = remainder
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
    
    ; Check signs and make positive
    xor edi, edi        ; quotient sign
    xor esi, esi        ; remainder sign
    
    ; Check dividend sign
    test edx, edx
    jns .check_divisor
    ; Negate dividend
    not eax
    not edx
    add eax, 1
    adc edx, 0
    inc edi             ; Toggle quotient sign
    inc esi             ; Remainder has dividend sign
    
.check_divisor:
    ; Check divisor sign
    test ecx, ecx
    jns .do_divide
    ; Negate divisor
    not ebx
    not ecx
    add ebx, 1
    adc ecx, 0
    inc edi             ; Toggle quotient sign
    
.do_divide:
    ; Save dividend for remainder calculation
    push edx
    push eax
    push ecx
    push ebx
    
    ; Perform unsigned 64-bit division
    call __udivmoddi4_internal
    
    ; Apply signs
    test edi, edi
    jz .check_remainder_sign
    ; Negate quotient
    not eax
    not edx
    add eax, 1
    adc edx, 0
    
.check_remainder_sign:
    test esi, esi
    jz .done
    ; Negate remainder
    not ebx
    not ecx
    add ebx, 1
    adc ecx, 0
    
.done:
    ; Return values in EDX:EAX (quotient) and ECX:EBX (remainder)
    pop esi
    pop esi
    pop esi
    pop esi
    
    pop ebx
    pop esi
    pop edi
    pop ebp
    ret

; Internal unsigned 64-bit division
; Input: EDX:EAX = dividend, ECX:EBX = divisor
; Output: EDX:EAX = quotient, ECX:EBX = remainder
__udivmoddi4_internal:
    push edi
    push esi
    
    ; Check for 32-bit divisor
    test ecx, ecx
    jnz .full_64bit_divide
    
    ; 32-bit divisor case
    mov ecx, eax
    mov eax, edx
    xor edx, edx
    div ebx
    push eax        ; Save high quotient
    mov eax, ecx
    div ebx
    mov ecx, edx    ; Remainder in ECX
    xor ebx, ebx    ; High remainder is 0
    pop edx         ; Restore high quotient
    jmp .div_done
    
.full_64bit_divide:
    ; Full 64-bit division using bit-by-bit algorithm
    push ebp
    mov edi, 64     ; Loop counter
    xor esi, esi    ; Remainder high
    xor ebp, ebp    ; Remainder low
    
.div_loop:
    ; Shift dividend left by 1
    shl eax, 1
    rcl edx, 1
    
    ; Shift remainder left by 1 and add new bit
    rcl ebp, 1
    rcl esi, 1
    
    ; Compare remainder with divisor
    cmp esi, ecx
    jb .next_bit
    ja .subtract
    cmp ebp, ebx
    jb .next_bit
    
.subtract:
    ; Subtract divisor from remainder
    sub ebp, ebx
    sbb esi, ecx
    ; Set quotient bit
    or eax, 1
    
.next_bit:
    dec edi
    jnz .div_loop
    
    mov ecx, esi    ; Remainder high
    mov ebx, ebp    ; Remainder low
    pop ebp
    
.div_done:
    pop esi
    pop edi
    ret

; Signed 64-bit division
; Stack: [divisor_low][divisor_high][dividend_low][dividend_high][ret_addr]
global __divdi3
__divdi3:
    push ebp
    mov ebp, esp
    
    ; Call divmod and ignore remainder
    push dword [ebp+20]
    push dword [ebp+16]
    push dword [ebp+12]
    push dword [ebp+8]
    call __divmoddi4
    add esp, 16
    
    pop ebp
    ret

; Unsigned 64-bit division
; Stack: [divisor_low][divisor_high][dividend_low][dividend_high][ret_addr]
global __udivdi3
__udivdi3:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    
    ; Load arguments
    mov eax, [ebp+8]    ; dividend low
    mov edx, [ebp+12]   ; dividend high
    mov ebx, [ebp+16]   ; divisor low
    mov ecx, [ebp+20]   ; divisor high
    
    call __udivmoddi4_internal
    
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

; Unsigned 64-bit division with remainder
; Stack: [divisor_low][divisor_high][dividend_low][dividend_high][ret_addr]
global __udivmoddi4
__udivmoddi4:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    
    ; Load arguments
    mov eax, [ebp+8]    ; dividend low
    mov edx, [ebp+12]   ; dividend high
    mov ebx, [ebp+16]   ; divisor low
    mov ecx, [ebp+20]   ; divisor high
    
    call __udivmoddi4_internal
    
    ; Return remainder in ECX:EBX as well
    mov [ebp+16], ebx   ; Store remainder low
    mov [ebp+20], ecx   ; Store remainder high
    
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

; Signed 64-bit modulo
global __moddi3
__moddi3:
    push ebp
    mov ebp, esp
    
    ; Call divmod and return remainder
    push dword [ebp+20]
    push dword [ebp+16]
    push dword [ebp+12]
    push dword [ebp+8]
    call __divmoddi4
    add esp, 16
    
    ; Move remainder to result
    mov eax, ebx
    mov edx, ecx
    
    pop ebp
    ret

; Unsigned 64-bit modulo
global __umoddi3
__umoddi3:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    
    ; Load arguments
    mov eax, [ebp+8]    ; dividend low
    mov edx, [ebp+12]   ; dividend high
    mov ebx, [ebp+16]   ; divisor low
    mov ecx, [ebp+20]   ; divisor high
    
    call __udivmoddi4_internal
    
    ; Return remainder
    mov eax, ebx
    mov edx, ecx
    
    pop edi
    pop esi
    pop ebx
    pop ebp
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