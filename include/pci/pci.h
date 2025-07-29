#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// PCI Configuration Space Registers
#define PCI_CONFIG_ADDRESS      0xCF8
#define PCI_CONFIG_DATA         0xCFC

// PCI Configuration Space Offsets
#define PCI_VENDOR_ID           0x00
#define PCI_DEVICE_ID           0x02
#define PCI_COMMAND             0x04
#define PCI_STATUS              0x06
#define PCI_REVISION_ID         0x08
#define PCI_PROG_IF             0x09
#define PCI_SUBCLASS            0x0A
#define PCI_CLASS               0x0B
#define PCI_CACHE_LINE_SIZE     0x0C
#define PCI_LATENCY_TIMER       0x0D
#define PCI_HEADER_TYPE         0x0E
#define PCI_BIST                0x0F
#define PCI_BAR0                0x10
#define PCI_BAR1                0x14
#define PCI_BAR2                0x18
#define PCI_BAR3                0x1C
#define PCI_BAR4                0x20
#define PCI_BAR5                0x24
#define PCI_CARDBUS_CIS         0x28
#define PCI_SUBSYSTEM_VENDOR_ID 0x2C
#define PCI_SUBSYSTEM_ID        0x2E
#define PCI_EXPANSION_ROM_BASE  0x30
#define PCI_CAPABILITIES        0x34
#define PCI_INTERRUPT_LINE      0x3C
#define PCI_INTERRUPT_PIN       0x3D
#define PCI_MIN_GRANT           0x3E
#define PCI_MAX_LATENCY         0x3F

// PCI Command Register Bits
#define PCI_COMMAND_IO          0x0001
#define PCI_COMMAND_MEMORY      0x0002
#define PCI_COMMAND_MASTER      0x0004
#define PCI_COMMAND_SPECIAL     0x0008
#define PCI_COMMAND_INVALIDATE  0x0010
#define PCI_COMMAND_VGA_PALETTE 0x0020
#define PCI_COMMAND_PARITY      0x0040
#define PCI_COMMAND_WAIT        0x0080
#define PCI_COMMAND_SERR        0x0100
#define PCI_COMMAND_FAST_BACK   0x0200
#define PCI_COMMAND_INTX_DISABLE 0x0400

// PCI Header Type
#define PCI_HEADER_TYPE_NORMAL  0x00
#define PCI_HEADER_TYPE_BRIDGE  0x01
#define PCI_HEADER_TYPE_CARDBUS 0x02
#define PCI_HEADER_TYPE_MASK    0x7F
#define PCI_HEADER_TYPE_MULTI   0x80

// PCI BAR Types
#define PCI_BAR_TYPE_MASK       0x01
#define PCI_BAR_TYPE_MEM        0x00
#define PCI_BAR_TYPE_IO         0x01
#define PCI_BAR_MEM_TYPE_MASK   0x06
#define PCI_BAR_MEM_TYPE_32     0x00
#define PCI_BAR_MEM_TYPE_64     0x04
#define PCI_BAR_MEM_PREFETCH    0x08

// Invalid Values
#define PCI_VENDOR_INVALID      0xFFFF
#define PCI_DEVICE_INVALID      0xFFFF

// PCI Device Structure
typedef struct pci_device {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision_id;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class_code;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t bist;
    uint32_t bar[6];
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint32_t expansion_rom_base;
    uint8_t capabilities_ptr;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;
    
    // Location on PCI bus
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    
    // Driver specific data
    void *driver_data;
    
    // Linked list
    struct pci_device *next;
} pci_device_t;

// PCI Driver Structure
typedef struct pci_driver {
    const char *name;
    const uint16_t *vendor_ids;
    const uint16_t *device_ids;
    int (*probe)(pci_device_t *dev);
    void (*remove)(pci_device_t *dev);
    struct pci_driver *next;
} pci_driver_t;

// Function Prototypes

// Basic PCI Configuration Access
uint32_t pci_config_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t pci_config_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint8_t pci_config_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void pci_config_write32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);
void pci_config_write16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value);
void pci_config_write8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint8_t value);

// PCI Device Management
void pci_init(void);
void pci_scan_bus(uint8_t bus);
void pci_scan_device(uint8_t bus, uint8_t device);
void pci_scan_function(uint8_t bus, uint8_t device, uint8_t function);
pci_device_t* pci_get_device(uint16_t vendor_id, uint16_t device_id);
pci_device_t* pci_get_device_by_class(uint8_t class_code, uint8_t subclass);
pci_device_t* pci_get_all_devices(void);

// PCI BAR Management
uint32_t pci_get_bar_size(pci_device_t *dev, uint8_t bar_num);
uint32_t pci_get_bar_address(pci_device_t *dev, uint8_t bar_num);
bool pci_bar_is_io(pci_device_t *dev, uint8_t bar_num);
bool pci_bar_is_memory(pci_device_t *dev, uint8_t bar_num);
bool pci_bar_is_64bit(pci_device_t *dev, uint8_t bar_num);
bool pci_bar_is_prefetchable(pci_device_t *dev, uint8_t bar_num);

// PCI Driver Management
void pci_register_driver(pci_driver_t *driver);
void pci_unregister_driver(pci_driver_t *driver);

// PCI Device Control
void pci_enable_device(pci_device_t *dev);
void pci_disable_device(pci_device_t *dev);
void pci_enable_bus_mastering(pci_device_t *dev);
void pci_disable_bus_mastering(pci_device_t *dev);
void pci_enable_interrupts(pci_device_t *dev);
void pci_disable_interrupts(pci_device_t *dev);

// MSI/MSI-X Support (Stub for future implementation)
bool pci_msi_capable(pci_device_t *dev);
bool pci_msix_capable(pci_device_t *dev);
int pci_enable_msi(pci_device_t *dev);
int pci_enable_msix(pci_device_t *dev);
void pci_disable_msi(pci_device_t *dev);
void pci_disable_msix(pci_device_t *dev);

// Utility Functions
const char* pci_get_vendor_name(uint16_t vendor_id);
const char* pci_get_device_name(uint16_t vendor_id, uint16_t device_id);
const char* pci_get_class_name(uint8_t class_code);
const char* pci_get_subclass_name(uint8_t class_code, uint8_t subclass);
void pci_print_device_info(pci_device_t *dev);
void pci_print_all_devices(void);

#ifdef __cplusplus
}
#endif