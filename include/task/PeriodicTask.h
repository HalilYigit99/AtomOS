#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t functionAddr;
    uint32_t interval; // in milliseconds
    uint32_t lastExecution; // timestamp of the last execution
    bool isActive; // whether the task is currently active
} PeriodicTask;

PeriodicTask* createPeriodicTask(uint32_t functionAddr, uint32_t interval);

void startPeriodicTask(PeriodicTask* task);
void stopPeriodicTask(PeriodicTask* task);

#ifdef __cplusplus
}
#endif
