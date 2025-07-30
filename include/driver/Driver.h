#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


typedef struct {
    char* name;          // Name of the driver
    bool enabled;       // Whether the driver is enabled
    size_t type;          // Type of the driver (e.g., device, filesystem)
    void (*init)(void); // Function pointer to the driver's initialization function
    void (*enable)(void); // Function pointer to enable the driver
    void (*disable)(void); // Function pointer to disable the driver
} Driver;

void sys_driver_register(Driver* driver);
void sys_driver_unregister(Driver* driver);

extern Driver ps2kbd_driver; // PS/2 keyboard driver instance

#ifdef __cplusplus
}
#endif
