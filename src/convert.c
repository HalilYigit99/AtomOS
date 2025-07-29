#include "convert.h"

static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static const char __attribute__((unused)) DIGITS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void reverseString(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

int numDigits(long long value, int base) {
    if (value == 0) return 1;
    
    int count = 0;
    if (value < 0) {
        count = 1; // for minus sign
        value = -value;
    }
    
    while (value != 0) {
        value /= base;
        count++;
    }
    return count;
}

int numDigitsUnsigned(unsigned long long value, int base) {
    if (value == 0) return 1;
    
    int count = 0;
    while (value != 0) {
        value /= base;
        count++;
    }
    return count;
}

char* itoa(int value, char* buffer, int base) {
    if (base < 2 || base > 36) {
        *buffer = '\0';
        return buffer;
    }
    
    char* ptr = buffer;
    bool isNegative = false;
    
    if (value < 0 && base == 10) {
        isNegative = true;
        value = -value;
    }
    
    // Convert to string in reverse order
    int i = 0;
    do {
        ptr[i++] = digits[value % base];
        value /= base;
    } while (value != 0);
    
    // Add negative sign if needed
    if (isNegative) {
        ptr[i++] = '-';
    }
    
    // Null terminate
    ptr[i] = '\0';
    
    // Reverse the string
    reverseString(ptr, i);
    
    return buffer;
}

char* ltoa(long value, char* buffer, int base) {
    if (base < 2 || base > 36) {
        *buffer = '\0';
        return buffer;
    }
    
    char* ptr = buffer;
    bool isNegative = false;
    
    if (value < 0 && base == 10) {
        isNegative = true;
        value = -value;
    }
    
    int i = 0;
    do {
        ptr[i++] = digits[value % base];
        value /= base;
    } while (value != 0);
    
    if (isNegative) {
        ptr[i++] = '-';
    }
    
    ptr[i] = '\0';
    reverseString(ptr, i);
    
    return buffer;
}

char* lltoa(long long value, char* buffer, int base) {
    if (base < 2 || base > 36) {
        *buffer = '\0';
        return buffer;
    }
    
    char* ptr = buffer;
    bool isNegative = false;
    
    if (value < 0 && base == 10) {
        isNegative = true;
        value = -value;
    }
    
    int i = 0;
    do {
        ptr[i++] = digits[value % base];
        value /= base;
    } while (value != 0);
    
    if (isNegative) {
        ptr[i++] = '-';
    }
    
    ptr[i] = '\0';
    reverseString(ptr, i);
    
    return buffer;
}

char* utoa(unsigned int value, char* buffer, int base) {
    if (base < 2 || base > 36) {
        *buffer = '\0';
        return buffer;
    }
    
    char* ptr = buffer;
    int i = 0;
    
    do {
        ptr[i++] = digits[value % base];
        value /= base;
    } while (value != 0);
    
    ptr[i] = '\0';
    reverseString(ptr, i);
    
    return buffer;
}

char* ultoa(unsigned long value, char* buffer, int base) {
    if (base < 2 || base > 36) {
        *buffer = '\0';
        return buffer;
    }
    
    char* ptr = buffer;
    int i = 0;
    
    do {
        ptr[i++] = digits[value % base];
        value /= base;
    } while (value != 0);
    
    ptr[i] = '\0';
    reverseString(ptr, i);
    
    return buffer;
}

char* ulltoa(unsigned long long value, char* buffer, int base) {
    if (base < 2 || base > 36) {
        *buffer = '\0';
        return buffer;
    }
    
    char* ptr = buffer;
    int i = 0;
    
    do {
        ptr[i++] = digits[value % base];
        value /= base;
    } while (value != 0);
    
    ptr[i] = '\0';
    reverseString(ptr, i);
    
    return buffer;
}

char* ptoa(void* ptr, char* buffer) {
    uintptr_t value = (uintptr_t)ptr;
    
    // Add "0x" prefix
    buffer[0] = '0';
    buffer[1] = 'x';
    
    // Convert to hex
    char* ptr_buf = buffer + 2;
    int i = 0;
    
    if (value == 0) {
        ptr_buf[i++] = '0';
    } else {
        while (value != 0) {
            ptr_buf[i++] = digits[value & 0xF];
            value >>= 4;
        }
    }
    
    ptr_buf[i] = '\0';
    reverseString(ptr_buf, i);
    
    return buffer;
}

// Basic floating point conversion (simple implementation)
char* ftoa(float value, char* buffer, int precision) {
    char* ptr = buffer;
    
    // Handle negative numbers
    if (value < 0) {
        *ptr++ = '-';
        value = -value;
    }
    
    // Get integer part
    int intPart = (int)value;
    float fractPart = value - intPart;
    
    // Convert integer part
    char intBuffer[32];
    itoa(intPart, intBuffer, 10);
    
    // Copy integer part
    char* intPtr = intBuffer;
    while (*intPtr) {
        *ptr++ = *intPtr++;
    }
    
    // Add decimal point
    *ptr++ = '.';
    
    // Convert fractional part
    if (precision <= 0) precision = 6; // Default precision
    
    for (int i = 0; i < precision; i++) {
        fractPart *= 10;
        int digit = (int)fractPart;
        *ptr++ = '0' + digit;
        fractPart -= digit;
    }
    
    *ptr = '\0';
    return buffer;
}

char* dtoa(double value, char* buffer, int precision) {
    char* ptr = buffer;
    
    // Handle special cases
    if (value != value) { // NaN
        ptr[0] = 'n'; ptr[1] = 'a'; ptr[2] = 'n'; ptr[3] = '\0';
        return buffer;
    }
    
    // Handle negative numbers
    if (value < 0) {
        *ptr++ = '-';
        value = -value;
    }
    
    // Handle infinity
    if (value > 1e308) {
        ptr[0] = 'i'; ptr[1] = 'n'; ptr[2] = 'f'; ptr[3] = '\0';
        return buffer;
    }
    
    // Get integer part
    long long intPart = (long long)value;
    double fractPart = value - intPart;
    
    // Convert integer part
    char intBuffer[64];
    lltoa(intPart, intBuffer, 10);
    
    // Copy integer part
    char* intPtr = intBuffer;
    while (*intPtr) {
        *ptr++ = *intPtr++;
    }
    
    // Add decimal point
    *ptr++ = '.';
    
    // Convert fractional part
    if (precision <= 0) precision = 6; // Default precision
    
    for (int i = 0; i < precision; i++) {
        fractPart *= 10;
        int digit = (int)fractPart;
        *ptr++ = '0' + digit;
        fractPart -= digit;
    }
    
    // Remove trailing zeros
    ptr--;
    while (ptr > buffer && *ptr == '0' && *(ptr-1) != '.') {
        ptr--;
    }
    ptr++;
    
    *ptr = '\0';
    return buffer;
}