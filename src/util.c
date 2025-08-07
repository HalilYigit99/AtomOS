#include <util.h>
#include <memory/memory.h>

extern uint32_t uptime_ms;

int util_basic_compare(const void* a, const void* b)
{
    return a == b;
}

void sleep(uint16_t ms) {
    uint32_t now = uptime_ms;
    uint32_t end = now + (uint32_t)ms;

    while (uptime_ms < end) {
        asm volatile ("nop");
    }
}
