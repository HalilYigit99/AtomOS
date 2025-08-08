#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

extern void kprintf(const char* fmt, ...);
extern void printf(const char* fmt, ...);

#ifdef __cplusplus
}
#endif
