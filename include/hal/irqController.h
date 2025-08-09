#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


typedef struct {
    void (*enable_irq)(uint8_t irq);
    void (*disable_irq)(uint8_t irq);
    void (*acknowledge_irq)(uint8_t irq);
    void (*set_irq_mask)(uint8_t irq, bool mask);
    void (*set_irq_priority)(uint8_t irq, uint8_t priority);
    void (*set_irq_handler)(uint8_t irq, void (*handler)(void));
    void (*set_irq_vector)(uint8_t irq, uint8_t vector);
} IRQController;

extern IRQController *irq_controller;

#ifdef __cplusplus
}
#endif
