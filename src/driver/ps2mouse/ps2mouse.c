#include <intel86.h>
#include <driver/Driver.h>

static bool enabled = false;

extern void ps2mouse_isr();

static void ps2mouse_init(void) {
    // Initialize PS/2 mouse hardware
    // This function should set up the necessary hardware registers and configurations

    

}

static void ps2mouse_enable(void) {
    // Enable the PS/2 mouse driver
    // This function should enable the mouse hardware and prepare it for input
    pic_unmask(12); // Unmask the PS/2 mouse interrupt (IRQ 12)
    enabled = true; // Set the driver as enabled
}

static void ps2mouse_disable(void) {
    // Disable the PS/2 mouse driver
    // This function should disable the mouse hardware and stop processing input
    pic_mask(12); // Mask the PS/2 mouse interrupt (IRQ 12)
    enabled = false; // Set the driver as disabled
}

static bool ps2mouse_enabled(void) {
    // Check if the PS/2 mouse driver is enabled
    return enabled;
}

void ps2mouse_isr_handler() {
    // Interrupt Service Routine for PS/2 mouse
    // This function should handle mouse input and update the mouse state
    // For example, read data from the PS/2 mouse port and process it



}

Driver ps2mouse_driver = {
    .name = "PS/2 Mouse Driver",
    .init = ps2mouse_init,
    .enable = ps2mouse_enable,
    .disable = ps2mouse_disable,
    .enabled = ps2mouse_enabled,
    .type = DRIVER_TYPE_HID
};

