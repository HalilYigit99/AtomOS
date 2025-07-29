#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int(*Open)();
    void(*Close)();

    int(*writeChar)(char c);
    int(*writeString)(const char* str);
    int(*writeBuffer)(const void* buffer, size_t size);

    int(*print)(const char* str);
    int(*printf)(const char* format, ...);

} OutputStream;

extern OutputStream* currentOutputStream; // Current output stream

#ifdef __cplusplus
}
#endif
