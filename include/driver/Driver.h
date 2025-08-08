#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    DRIVER_TYPE_ANY = 0,
    DRIVER_TYPE_HID = 1,        // Human Interface Device (e.g., keyboard, mouse)
    DRIVER_TYPE_STORAGE = 2,    // Storage devices (e.g., ATA, SATA)
    DRIVER_TYPE_NETWORK = 3,    // Network devices (e.g., Ethernet, Wi-Fi)
    DRIVER_TYPE_GRAPHICS = 4,   // Graphics devices (e.g., GPUs)
    DRIVER_TYPE_FILESYSTEM = 5, // Filesystem drivers
    DRIVER_TYPE_AUDIO = 6,      // Audio devices
} DriverType;

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
