#include <stream/VPrintf.h>
#include "convert.h"

// Printf format flags
typedef struct {
    bool leftAlign;      // '-' flag
    bool showSign;       // '+' flag  
    bool spaceSign;      // ' ' flag
    bool alternate;      // '#' flag
    bool zeroPad;        // '0' flag
    int width;           // Field width
    int precision;       // Precision
    bool hasPrecision;   // Precision specified
} FormatFlags;

static int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

static int parseNumber(const char** format) {
    int num = 0;
    while (**format >= '0' && **format <= '9') {
        num = num * 10 + (**format - '0');
        (*format)++;
    }
    return num;
}

static FormatFlags parseFlags(const char** format) {
    FormatFlags flags = {0};
    
    // Parse flags
    bool parsing = true;
    while (parsing) {
        switch (**format) {
            case '-': flags.leftAlign = true; (*format)++; break;
            case '+': flags.showSign = true; (*format)++; break;
            case ' ': flags.spaceSign = true; (*format)++; break;
            case '#': flags.alternate = true; (*format)++; break;
            case '0': flags.zeroPad = true; (*format)++; break;
            default: parsing = false; break;
        }
    }
    
    // Parse width
    if (**format >= '1' && **format <= '9') {
        flags.width = parseNumber(format);
    } else if (**format == '*') {
        flags.width = -1; // Will be read from va_list
        (*format)++;
    }
    
    // Parse precision
    if (**format == '.') {
        (*format)++;
        flags.hasPrecision = true;
        if (**format >= '0' && **format <= '9') {
            flags.precision = parseNumber(format);
        } else if (**format == '*') {
            flags.precision = -1; // Will be read from va_list
            (*format)++;
        } else {
            flags.precision = 0;
        }
    }
    
    return flags;
}

static void printPadding(void(*putChar)(char), int count, char padChar) {
    for (int i = 0; i < count; i++) {
        putChar(padChar);
    }
}

static void printString(void(*putChar)(char), const char* str, FormatFlags flags) {
    if (!str) str = "(null)";
    
    int len = strlen(str);
    if (flags.hasPrecision && flags.precision < len) {
        len = flags.precision;
    }
    
    int padding = flags.width - len;
    
    if (!flags.leftAlign && padding > 0) {
        printPadding(putChar, padding, ' ');
    }
    
    for (int i = 0; i < len; i++) {
        putChar(str[i]);
    }
    
    if (flags.leftAlign && padding > 0) {
        printPadding(putChar, padding, ' ');
    }
}

static void printNumber(void(*putChar)(char), long long value, int base, bool uppercase, FormatFlags flags) {
    char buffer[66]; // Enough for 64-bit binary + sign + null
    lltoa(value, buffer, base);
    
    // Handle uppercase for hex
    if (uppercase && base == 16) {
        for (int i = 0; buffer[i]; i++) {
            if (buffer[i] >= 'a' && buffer[i] <= 'f') {
                buffer[i] = buffer[i] - 'a' + 'A';
            }
        }
    }
    
    int len = strlen(buffer);
    bool hasSign = buffer[0] == '-';
    
    // Calculate prefix length
    int prefixLen = 0;
    if (hasSign) prefixLen = 1;
    else if (flags.showSign) prefixLen = 1;
    else if (flags.spaceSign) prefixLen = 1;
    
    if (flags.alternate) {
        if (base == 8 && buffer[0] != '0') prefixLen += 1;
        else if (base == 16 && value != 0) prefixLen += 2;
    }
    
    int totalLen = len + (hasSign ? 0 : prefixLen);
    int padding = flags.width - totalLen;
    
    // Print left padding (spaces)
    if (!flags.leftAlign && !flags.zeroPad && padding > 0) {
        printPadding(putChar, padding, ' ');
    }
    
    // Print prefix
    if (hasSign) {
        putChar('-');
    } else if (flags.showSign) {
        putChar('+');
    } else if (flags.spaceSign) {
        putChar(' ');
    }
    
    if (flags.alternate) {
        if (base == 8 && buffer[0] != '0') {
            putChar('0');
        } else if (base == 16 && value != 0) {
            putChar('0');
            putChar(uppercase ? 'X' : 'x');
        }
    }
    
    // Print zero padding
    if (!flags.leftAlign && flags.zeroPad && padding > 0) {
        printPadding(putChar, padding, '0');
    }
    
    // Print number (skip sign if already printed)
    const char* numStart = hasSign ? buffer + 1 : buffer;
    while (*numStart) {
        putChar(*numStart++);
    }
    
    // Print right padding
    if (flags.leftAlign && padding > 0) {
        printPadding(putChar, padding, ' ');
    }
}

static void printUnsignedNumber(void(*putChar)(char), unsigned long long value, int base, bool uppercase, FormatFlags flags) {
    char buffer[65]; // Enough for 64-bit binary + null
    ulltoa(value, buffer, base);
    
    // Handle uppercase for hex
    if (uppercase && base == 16) {
        for (int i = 0; buffer[i]; i++) {
            if (buffer[i] >= 'a' && buffer[i] <= 'f') {
                buffer[i] = buffer[i] - 'a' + 'A';
            }
        }
    }
    
    int len = strlen(buffer);
    int prefixLen = 0;
    
    if (flags.alternate) {
        if (base == 8 && buffer[0] != '0') prefixLen = 1;
        else if (base == 16 && value != 0) prefixLen = 2;
    }
    
    int totalLen = len + prefixLen;
    int padding = flags.width - totalLen;
    
    // Print left padding
    if (!flags.leftAlign && !flags.zeroPad && padding > 0) {
        printPadding(putChar, padding, ' ');
    }
    
    // Print prefix
    if (flags.alternate) {
        if (base == 8 && buffer[0] != '0') {
            putChar('0');
        } else if (base == 16 && value != 0) {
            putChar('0');
            putChar(uppercase ? 'X' : 'x');
        }
    }
    
    // Print zero padding
    if (!flags.leftAlign && flags.zeroPad && padding > 0) {
        printPadding(putChar, padding, '0');
    }
    
    // Print number
    const char* ptr = buffer;
    while (*ptr) {
        putChar(*ptr++);
    }
    
    // Print right padding
    if (flags.leftAlign && padding > 0) {
        printPadding(putChar, padding, ' ');
    }
}

static void printFloat(void(*putChar)(char), double value, FormatFlags flags) {
    char buffer[128];
    int precision = flags.hasPrecision ? flags.precision : 6;
    
    // Convert to string
    dtoa(value, buffer, precision);
    
    // Print as string with formatting
    printString(putChar, buffer, flags);
}

void vprintf(void(*putChar)(char), const char* format, va_list list) {
    while (*format) {
        if (*format != '%') {
            putChar(*format++);
            continue;
        }
        
        format++; // Skip '%'
        
        // Check for %%
        if (*format == '%') {
            putChar('%');
            format++;
            continue;
        }
        
        // Parse flags
        FormatFlags flags = parseFlags(&format);
        
        // Get width from va_list if needed
        if (flags.width == -1) {
            flags.width = va_arg(list, int);
            if (flags.width < 0) {
                flags.leftAlign = true;
                flags.width = -flags.width;
            }
        }
        
        // Get precision from va_list if needed
        if (flags.hasPrecision && flags.precision == -1) {
            flags.precision = va_arg(list, int);
            if (flags.precision < 0) {
                flags.hasPrecision = false;
            }
        }
        
        // Parse length modifiers
        int length = 0; // 0=int, 1=long, 2=long long
        bool isShort = false;
        bool isChar = false;
        
        if (*format == 'h') {
            format++;
            if (*format == 'h') {
                isChar = true;
                format++;
            } else {
                isShort = true;
            }
        } else if (*format == 'l') {
            format++;
            if (*format == 'l') {
                length = 2;
                format++;
            } else {
                length = 1;
            }
        } else if (*format == 'z' || *format == 't') {
            length = sizeof(size_t) == sizeof(long) ? 1 : 2;
            format++;
        }
        
        // Handle format specifier
        switch (*format) {
            case 'd':
            case 'i': {
                long long value;
                if (isChar) value = (char)va_arg(list, int);
                else if (isShort) value = (short)va_arg(list, int);
                else if (length == 0) value = va_arg(list, int);
                else if (length == 1) value = va_arg(list, long);
                else value = va_arg(list, long long);
                
                printNumber(putChar, value, 10, false, flags);
                break;
            }
            
            case 'u': {
                unsigned long long value;
                if (isChar) value = (unsigned char)va_arg(list, unsigned int);
                else if (isShort) value = (unsigned short)va_arg(list, unsigned int);
                else if (length == 0) value = va_arg(list, unsigned int);
                else if (length == 1) value = va_arg(list, unsigned long);
                else value = va_arg(list, unsigned long long);
                
                printUnsignedNumber(putChar, value, 10, false, flags);
                break;
            }
            
            case 'o': {
                unsigned long long value;
                if (isChar) value = (unsigned char)va_arg(list, unsigned int);
                else if (isShort) value = (unsigned short)va_arg(list, unsigned int);
                else if (length == 0) value = va_arg(list, unsigned int);
                else if (length == 1) value = va_arg(list, unsigned long);
                else value = va_arg(list, unsigned long long);
                
                printUnsignedNumber(putChar, value, 8, false, flags);
                break;
            }
            
            case 'x':
            case 'X': {
                unsigned long long value;
                if (isChar) value = (unsigned char)va_arg(list, unsigned int);
                else if (isShort) value = (unsigned short)va_arg(list, unsigned int);
                else if (length == 0) value = va_arg(list, unsigned int);
                else if (length == 1) value = va_arg(list, unsigned long);
                else value = va_arg(list, unsigned long long);
                
                printUnsignedNumber(putChar, value, 16, *format == 'X', flags);
                break;
            }
            
            case 'c': {
                char c = (char)va_arg(list, int);
                if (!flags.leftAlign && flags.width > 1) {
                    printPadding(putChar, flags.width - 1, ' ');
                }
                putChar(c);
                if (flags.leftAlign && flags.width > 1) {
                    printPadding(putChar, flags.width - 1, ' ');
                }
                break;
            }
            
            case 's': {
                const char* str = va_arg(list, const char*);
                printString(putChar, str, flags);
                break;
            }
            
            case 'p': {
                void* ptr = va_arg(list, void*);
                char buffer[20];
                ptoa(ptr, buffer);
                printString(putChar, buffer, flags);
                break;
            }
            
            case 'f':
            case 'F': {
                double value = va_arg(list, double);
                printFloat(putChar, value, flags);
                break;
            }
            
            case 'e':
            case 'E':
            case 'g':
            case 'G': {
                // For now, treat as 'f'
                double value = va_arg(list, double);
                printFloat(putChar, value, flags);
                break;
            }
            
            case 'n': {
                // Number of characters written so far
                // Not implemented for security reasons
                break;
            }
            
            default:
                // Unknown format, just print it
                putChar('%');
                putChar(*format);
                break;
        }
        
        format++;
    }
}