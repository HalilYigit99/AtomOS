#include <driver/Driver.h>
#include <list.h>

List* driver_list = NULL; // Global list to hold registered drivers

void sys_driver_register(Driver* driver) {

    if (!driver_list) {
        driver_list = list_create();
        if (!driver_list) {
            return; // Failed to create driver list
        }
    }

    if (!driver || !driver->name || !driver->init) {
        return; // Invalid driver
    }
    
    // Register the driver in the system (implementation-specific)
    // This could involve adding it to a global list of drivers, etc.
    if (list_add(driver_list, driver) != 0) {
        return; // Failed to add driver to the list
    }

    // Initialize the driver
    if (driver->init) driver->init();

    // Enable the driver if it has an enable function
    if (driver->enable) {
        driver->enable();
    }

}

void sys_driver_unregister(Driver* driver) {

    if (!driver_list || !driver) {
        return; // No driver list or invalid driver
    }

    if (!driver || !driver->name) {
        return; // Invalid driver
    }
    
    // Disable the driver if it is enabled
    if (driver->enabled && driver->disable) {
        driver->disable();
    }
    
    // Unregister the driver from the system (implementation-specific)
    // This could involve removing it from a global list of drivers, etc.
    if (list_remove(driver_list, driver) != 0) {
        return; // Failed to remove driver from the list
    }
}

