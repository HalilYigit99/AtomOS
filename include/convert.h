#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// String manipulation
void reverseString(char* str, int length);

// Number digit counting
int numDigits(long long value, int base);
int numDigitsUnsigned(unsigned long long value, int base);

// Integer to string conversions
char* itoa(int value, char* buffer, int base);
char* ltoa(long value, char* buffer, int base);
char* lltoa(long long value, char* buffer, int base);

// Unsigned integer to string conversions
char* utoa(unsigned int value, char* buffer, int base);
char* ultoa(unsigned long value, char* buffer, int base);
char* ulltoa(unsigned long long value, char* buffer, int base);

// Pointer to string conversion
char* ptoa(void* ptr, char* buffer);

// Floating point to string conversions
char* ftoa(float value, char* buffer, int precision);
char* dtoa(double value, char* buffer, int precision);

#ifdef __cplusplus
}
#endif