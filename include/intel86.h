#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <io.h>

void intel86_idt_set_entry(size_t index, uint32_t base, uint16_t selector, uint8_t type_attr);

void pic_mask(unsigned char id);
void pic_unmask(unsigned char id);



#ifdef __cplusplus
}
#endif
