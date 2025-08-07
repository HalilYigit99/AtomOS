#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

int util_basic_compare(const void* a, const void* b);

void sleep(uint16_t ms);

#ifdef __cplusplus
};
#endif
