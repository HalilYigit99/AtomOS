#include <pci/pci.h>
#include <stddef.h>
#include <memory/memory.h>
#include <io.h>
#include <print.h>

// Assuming these are provided by your kernel
extern uint32_t inl(uint16_t port);
extern void outl(uint16_t port, uint32_t value);
extern uint16_t inw(uint16_t port);
extern void outw(uint16_t port, uint16_t value);
extern uint8_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t value);
extern void* kmalloc(size_t size);
extern void kfree(void* ptr);
extern void kprintf(const char* fmt, ...);

// Global PCI device list
static pci_device_t* pci_devices = NULL;
static pci_driver_t* pci_drivers = NULL;

// PCI Configuration Access Functions
static uint32_t pci_make_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    return (uint32_t)((bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC) | 0x80000000);
}

uint32_t pci_config_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = pci_make_address(bus, device, function, offset);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_config_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t data = pci_config_read32(bus, device, function, offset & 0xFC);
    return (uint16_t)((data >> ((offset & 2) * 8)) & 0xFFFF);
}

uint8_t pci_config_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t data = pci_config_read32(bus, device, function, offset & 0xFC);
    return (uint8_t)((data >> ((offset & 3) * 8)) & 0xFF);
}

void pci_config_write32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = pci_make_address(bus, device, function, offset);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

void pci_config_write16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value) {
    uint32_t data = pci_config_read32(bus, device, function, offset & 0xFC);
    data &= ~(0xFFFF << ((offset & 2) * 8));
    data |= (uint32_t)value << ((offset & 2) * 8);
    pci_config_write32(bus, device, function, offset & 0xFC, data);
}

void pci_config_write8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint8_t value) {
    uint32_t data = pci_config_read32(bus, device, function, offset & 0xFC);
    data &= ~(0xFF << ((offset & 3) * 8));
    data |= (uint32_t)value << ((offset & 3) * 8);
    pci_config_write32(bus, device, function, offset & 0xFC, data);
}

// Device scanning and initialization
static pci_device_t* pci_create_device(uint8_t bus, uint8_t device, uint8_t function) {
    pci_device_t* dev = (pci_device_t*)kmalloc(sizeof(pci_device_t));
    if (!dev) return NULL;
    
    memset(dev, 0, sizeof(pci_device_t));
    
    dev->bus = bus;
    dev->device = device;
    dev->function = function;
    
    // Read basic configuration
    dev->vendor_id = pci_config_read16(bus, device, function, PCI_VENDOR_ID);
    dev->device_id = pci_config_read16(bus, device, function, PCI_DEVICE_ID);
    dev->command = pci_config_read16(bus, device, function, PCI_COMMAND);
    dev->status = pci_config_read16(bus, device, function, PCI_STATUS);
    
    dev->revision_id = pci_config_read8(bus, device, function, PCI_REVISION_ID);
    dev->prog_if = pci_config_read8(bus, device, function, PCI_PROG_IF);
    dev->subclass = pci_config_read8(bus, device, function, PCI_SUBCLASS);
    dev->class_code = pci_config_read8(bus, device, function, PCI_CLASS);
    
    dev->cache_line_size = pci_config_read8(bus, device, function, PCI_CACHE_LINE_SIZE);
    dev->latency_timer = pci_config_read8(bus, device, function, PCI_LATENCY_TIMER);
    dev->header_type = pci_config_read8(bus, device, function, PCI_HEADER_TYPE);
    dev->bist = pci_config_read8(bus, device, function, PCI_BIST);
    
    // Read BARs for normal devices
    if ((dev->header_type & PCI_HEADER_TYPE_MASK) == PCI_HEADER_TYPE_NORMAL) {
        for (int i = 0; i < 6; i++) {
            dev->bar[i] = pci_config_read32(bus, device, function, PCI_BAR0 + (i * 4));
        }
        
        dev->subsystem_vendor_id = pci_config_read16(bus, device, function, PCI_SUBSYSTEM_VENDOR_ID);
        dev->subsystem_id = pci_config_read16(bus, device, function, PCI_SUBSYSTEM_ID);
        dev->expansion_rom_base = pci_config_read32(bus, device, function, PCI_EXPANSION_ROM_BASE);
        dev->capabilities_ptr = pci_config_read8(bus, device, function, PCI_CAPABILITIES);
        dev->interrupt_line = pci_config_read8(bus, device, function, PCI_INTERRUPT_LINE);
        dev->interrupt_pin = pci_config_read8(bus, device, function, PCI_INTERRUPT_PIN);
        dev->min_grant = pci_config_read8(bus, device, function, PCI_MIN_GRANT);
        dev->max_latency = pci_config_read8(bus, device, function, PCI_MAX_LATENCY);
    }
    
    // Add to device list
    dev->next = pci_devices;
    pci_devices = dev;
    
    return dev;
}

void pci_scan_function(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t vendor_id = pci_config_read16(bus, device, function, PCI_VENDOR_ID);
    if (vendor_id == PCI_VENDOR_INVALID) return;
    
    pci_device_t* dev = pci_create_device(bus, device, function);
    if (!dev) return;
    
    // Check if this is a PCI bridge
    if ((dev->class_code == 0x06) && (dev->subclass == 0x04)) {
        // PCI-to-PCI bridge, scan secondary bus
        uint8_t secondary_bus = pci_config_read8(bus, device, function, 0x19);
        pci_scan_bus(secondary_bus);
    }
    
    // Try to match with registered drivers
    pci_driver_t* driver = pci_drivers;
    while (driver) {
        bool match = false;
        
        // Check vendor/device ID match
        if (driver->vendor_ids && driver->device_ids) {
            for (int i = 0; driver->vendor_ids[i] != 0; i++) {
                if (driver->vendor_ids[i] == dev->vendor_id) {
                    for (int j = 0; driver->device_ids[j] != 0; j++) {
                        if (driver->device_ids[j] == dev->device_id) {
                            match = true;
                            break;
                        }
                    }
                    if (match) break;
                }
            }
        }
        
        if (match && driver->probe) {
            if (driver->probe(dev) == 0) {
                kprintf("PCI: %s driver claimed device %04x:%04x\n", 
                       driver->name, dev->vendor_id, dev->device_id);
            }
        }
        
        driver = driver->next;
    }
}

void pci_scan_device(uint8_t bus, uint8_t device) {
    uint16_t vendor_id = pci_config_read16(bus, device, 0, PCI_VENDOR_ID);
    if (vendor_id == PCI_VENDOR_INVALID) return;
    
    pci_scan_function(bus, device, 0);
    
    // Check if multi-function device
    uint8_t header_type = pci_config_read8(bus, device, 0, PCI_HEADER_TYPE);
    if (header_type & PCI_HEADER_TYPE_MULTI) {
        for (uint8_t function = 1; function < 8; function++) {
            vendor_id = pci_config_read16(bus, device, function, PCI_VENDOR_ID);
            if (vendor_id != PCI_VENDOR_INVALID) {
                pci_scan_function(bus, device, function);
            }
        }
    }
}

void pci_scan_bus(uint8_t bus) {
    for (uint8_t device = 0; device < 32; device++) {
        pci_scan_device(bus, device);
    }
}

void pci_init(void) {
    kprintf("PCI: Initializing PCI subsystem...\n");
    
    // First check if PCI is supported by trying to read from bus 0, device 0
    uint32_t id = pci_config_read32(0, 0, 0, 0);
    if (id == 0xFFFFFFFF || id == 0x00000000) {
        kprintf("PCI: No PCI devices found or PCI not supported!\n");
        kprintf("PCI: Check if running in an emulator with PCI support enabled\n");
        return;
    }
    
    // Method 1: Check if there's a host bridge on bus 0
    pci_scan_bus(0);
    
    // Method 2: If no devices found, try brute force scan of first few buses
    if (!pci_devices) {
        for (uint8_t bus = 1; bus < 8; bus++) {
            uint16_t vendor = pci_config_read16(bus, 0, 0, PCI_VENDOR_ID);
            if (vendor != 0xFFFF && vendor != 0x0000) {
                kprintf("PCI: Found device on bus %d\n", bus);
                pci_scan_bus(bus);
            }
        }
    }
    
    // Count devices
    int count = 0;
    pci_device_t* dev = pci_devices;
    while (dev) {
        count++;
        dev = dev->next;
    }
    
}

// Device lookup functions
pci_device_t* pci_get_device(uint16_t vendor_id, uint16_t device_id) {
    pci_device_t* dev = pci_devices;
    while (dev) {
        if (dev->vendor_id == vendor_id && dev->device_id == device_id) {
            return dev;
        }
        dev = dev->next;
    }
    return NULL;
}

pci_device_t* pci_get_device_by_class(uint8_t class_code, uint8_t subclass) {
    pci_device_t* dev = pci_devices;
    while (dev) {
        if (dev->class_code == class_code && dev->subclass == subclass) {
            return dev;
        }
        dev = dev->next;
    }
    return NULL;
}

pci_device_t* pci_get_all_devices(void) {
    return pci_devices;
}

// BAR management functions
uint32_t pci_get_bar_size(pci_device_t* dev, uint8_t bar_num) {
    if (bar_num >= 6) return 0;
    
    uint8_t offset = PCI_BAR0 + (bar_num * 4);
    uint32_t original = pci_config_read32(dev->bus, dev->device, dev->function, offset);
    
    // Write all 1s to get size
    pci_config_write32(dev->bus, dev->device, dev->function, offset, 0xFFFFFFFF);
    uint32_t size = pci_config_read32(dev->bus, dev->device, dev->function, offset);
    
    // Restore original value
    pci_config_write32(dev->bus, dev->device, dev->function, offset, original);
    
    if (size == 0) return 0;
    
    // Calculate actual size
    if (original & PCI_BAR_TYPE_IO) {
        size &= 0xFFFFFFFC;
        size = (~size) + 1;
    } else {
        size &= 0xFFFFFFF0;
        size = (~size) + 1;
    }
    
    return size;
}

uint32_t pci_get_bar_address(pci_device_t* dev, uint8_t bar_num) {
    if (bar_num >= 6) return 0;
    
    uint32_t bar = dev->bar[bar_num];
    if (bar & PCI_BAR_TYPE_IO) {
        return bar & 0xFFFFFFFC;
    } else {
        return bar & 0xFFFFFFF0;
    }
}

bool pci_bar_is_io(pci_device_t* dev, uint8_t bar_num) {
    if (bar_num >= 6) return false;
    return (dev->bar[bar_num] & PCI_BAR_TYPE_MASK) == PCI_BAR_TYPE_IO;
}

bool pci_bar_is_memory(pci_device_t* dev, uint8_t bar_num) {
    if (bar_num >= 6) return false;
    return (dev->bar[bar_num] & PCI_BAR_TYPE_MASK) == PCI_BAR_TYPE_MEM;
}

bool pci_bar_is_64bit(pci_device_t* dev, uint8_t bar_num) {
    if (bar_num >= 6) return false;
    if (!(dev->bar[bar_num] & PCI_BAR_TYPE_MASK)) {
        return (dev->bar[bar_num] & PCI_BAR_MEM_TYPE_MASK) == PCI_BAR_MEM_TYPE_64;
    }
    return false;
}

bool pci_bar_is_prefetchable(pci_device_t* dev, uint8_t bar_num) {
    if (bar_num >= 6) return false;
    if (!(dev->bar[bar_num] & PCI_BAR_TYPE_MASK)) {
        return (dev->bar[bar_num] & PCI_BAR_MEM_PREFETCH) != 0;
    }
    return false;
}

// PCI BAR I/O Operations
void pci_bar_write8(pci_device_t* dev, uint8_t bar_num, uint32_t offset, uint8_t value) {
    if (bar_num >= 6) return;
    
    uint32_t bar_addr = pci_get_bar_address(dev, bar_num);
    if (bar_addr == 0) return;
    
    if (pci_bar_is_io(dev, bar_num)) {
        // I/O BAR - use port I/O
        outb(bar_addr + offset, value);
    } else {
        // Memory BAR - use memory mapped I/O
        volatile uint8_t* mem_addr = (volatile uint8_t*)(bar_addr + offset);
        *mem_addr = value;
    }
}

void pci_bar_write16(pci_device_t* dev, uint8_t bar_num, uint32_t offset, uint16_t value) {
    if (bar_num >= 6) return;
    
    uint32_t bar_addr = pci_get_bar_address(dev, bar_num);
    if (bar_addr == 0) return;
    
    if (pci_bar_is_io(dev, bar_num)) {
        // I/O BAR - use port I/O
        outw(bar_addr + offset, value);
    } else {
        // Memory BAR - use memory mapped I/O
        volatile uint16_t* mem_addr = (volatile uint16_t*)(bar_addr + offset);
        *mem_addr = value;
    }
}

void pci_bar_write32(pci_device_t* dev, uint8_t bar_num, uint32_t offset, uint32_t value) {
    if (bar_num >= 6) return;
    
    uint32_t bar_addr = pci_get_bar_address(dev, bar_num);
    if (bar_addr == 0) return;
    
    if (pci_bar_is_io(dev, bar_num)) {
        // I/O BAR - use port I/O
        outl(bar_addr + offset, value);
    } else {
        // Memory BAR - use memory mapped I/O
        volatile uint32_t* mem_addr = (volatile uint32_t*)(bar_addr + offset);
        *mem_addr = value;
    }
}

uint8_t pci_bar_read8(pci_device_t* dev, uint8_t bar_num, uint32_t offset) {
    if (bar_num >= 6) return 0;
    
    uint32_t bar_addr = pci_get_bar_address(dev, bar_num);
    if (bar_addr == 0) return 0;
    
    if (pci_bar_is_io(dev, bar_num)) {
        // I/O BAR - use port I/O
        return inb(bar_addr + offset);
    } else {
        // Memory BAR - use memory mapped I/O
        volatile uint8_t* mem_addr = (volatile uint8_t*)(bar_addr + offset);
        return *mem_addr;
    }
}

uint16_t pci_bar_read16(pci_device_t* dev, uint8_t bar_num, uint32_t offset) {
    if (bar_num >= 6) return 0;
    
    uint32_t bar_addr = pci_get_bar_address(dev, bar_num);
    if (bar_addr == 0) return 0;
    
    if (pci_bar_is_io(dev, bar_num)) {
        // I/O BAR - use port I/O
        return inw(bar_addr + offset);
    } else {
        // Memory BAR - use memory mapped I/O
        volatile uint16_t* mem_addr = (volatile uint16_t*)(bar_addr + offset);
        return *mem_addr;
    }
}

uint32_t pci_bar_read32(pci_device_t* dev, uint8_t bar_num, uint32_t offset) {
    if (bar_num >= 6) return 0;
    
    uint32_t bar_addr = pci_get_bar_address(dev, bar_num);
    if (bar_addr == 0) return 0;
    
    if (pci_bar_is_io(dev, bar_num)) {
        // I/O BAR - use port I/O
        return inl(bar_addr + offset);
    } else {
        // Memory BAR - use memory mapped I/O
        volatile uint32_t* mem_addr = (volatile uint32_t*)(bar_addr + offset);
        return *mem_addr;
    }
}

// Driver management
void pci_register_driver(pci_driver_t* driver) {
    if (!driver) return;
    
    driver->next = pci_drivers;
    pci_drivers = driver;
    
    // Try to match existing devices
    pci_device_t* dev = pci_devices;
    while (dev) {
        bool match = false;
        
        if (driver->vendor_ids && driver->device_ids) {
            for (int i = 0; driver->vendor_ids[i] != 0; i++) {
                if (driver->vendor_ids[i] == dev->vendor_id) {
                    for (int j = 0; driver->device_ids[j] != 0; j++) {
                        if (driver->device_ids[j] == dev->device_id) {
                            match = true;
                            break;
                        }
                    }
                    if (match) break;
                }
            }
        }
        
        if (match && driver->probe) {
            if (driver->probe(dev) == 0) {
                kprintf("PCI: %s driver claimed device %04x:%04x\n", 
                       driver->name, dev->vendor_id, dev->device_id);
            }
        }
        
        dev = dev->next;
    }
}

void pci_unregister_driver(pci_driver_t* driver) {
    if (!driver) return;
    
    // Remove from driver list
    if (pci_drivers == driver) {
        pci_drivers = driver->next;
    } else {
        pci_driver_t* prev = pci_drivers;
        while (prev && prev->next != driver) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = driver->next;
        }
    }
    
    // Call remove function for all devices using this driver
    pci_device_t* dev = pci_devices;
    while (dev) {
        if (dev->driver_data && driver->remove) {
            driver->remove(dev);
        }
        dev = dev->next;
    }
}

// Device control functions
void pci_enable_device(pci_device_t* dev) {
    uint16_t command = pci_config_read16(dev->bus, dev->device, dev->function, PCI_COMMAND);
    command |= PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
    pci_config_write16(dev->bus, dev->device, dev->function, PCI_COMMAND, command);
    dev->command = command;
}

void pci_disable_device(pci_device_t* dev) {
    uint16_t command = pci_config_read16(dev->bus, dev->device, dev->function, PCI_COMMAND);
    command &= ~(PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
    pci_config_write16(dev->bus, dev->device, dev->function, PCI_COMMAND, command);
    dev->command = command;
}

void pci_enable_bus_mastering(pci_device_t* dev) {
    uint16_t command = pci_config_read16(dev->bus, dev->device, dev->function, PCI_COMMAND);
    command |= PCI_COMMAND_MASTER;
    pci_config_write16(dev->bus, dev->device, dev->function, PCI_COMMAND, command);
    dev->command = command;
}

void pci_disable_bus_mastering(pci_device_t* dev) {
    uint16_t command = pci_config_read16(dev->bus, dev->device, dev->function, PCI_COMMAND);
    command &= ~PCI_COMMAND_MASTER;
    pci_config_write16(dev->bus, dev->device, dev->function, PCI_COMMAND, command);
    dev->command = command;
}

void pci_enable_interrupts(pci_device_t* dev) {
    uint16_t command = pci_config_read16(dev->bus, dev->device, dev->function, PCI_COMMAND);
    command &= ~PCI_COMMAND_INTX_DISABLE;
    pci_config_write16(dev->bus, dev->device, dev->function, PCI_COMMAND, command);
    dev->command = command;
}

void pci_disable_interrupts(pci_device_t* dev) {
    uint16_t command = pci_config_read16(dev->bus, dev->device, dev->function, PCI_COMMAND);
    command |= PCI_COMMAND_INTX_DISABLE;
    pci_config_write16(dev->bus, dev->device, dev->function, PCI_COMMAND, command);
    dev->command = command;
}

// MSI/MSI-X support stubs
bool pci_msi_capable(pci_device_t* dev) {
    // TODO: Implement capability list parsing
    return false;
}

bool pci_msix_capable(pci_device_t* dev) {
    // TODO: Implement capability list parsing
    return false;
}

int pci_enable_msi(pci_device_t* dev) {
    // TODO: Implement MSI enable
    return -1;
}

int pci_enable_msix(pci_device_t* dev) {
    // TODO: Implement MSI-X enable
    return -1;
}

void pci_disable_msi(pci_device_t* dev) {
    // TODO: Implement MSI disable
}

void pci_disable_msix(pci_device_t* dev) {
    // TODO: Implement MSI-X disable
}

// Utility functions
const char* pci_get_vendor_name(uint16_t vendor_id) {
    switch (vendor_id) {
        case 0x8086: return "Intel Corporation";
        case 0x1022: return "AMD";
        case 0x10DE: return "NVIDIA Corporation";
        case 0x10EC: return "Realtek Semiconductor";
        case 0x1002: return "ATI Technologies";
        case 0x14E4: return "Broadcom Corporation";
        case 0x168C: return "Atheros Communications";
        case 0x1106: return "VIA Technologies";
        case 0x10B7: return "3Com Corporation";
        case 0x1011: return "Digital Equipment Corporation";
        case 0x9005: return "Adaptec";
        case 0x1000: return "LSI Logic";
        case 0x15AD: return "VMware";
        case 0x1AF4: return "Red Hat (VirtIO)";
        case 0x1234: return "QEMU";
        case 0x80EE: return "VirtualBox";
        default: return "Unknown Vendor";
    }
}

const char* pci_get_device_name(uint16_t vendor_id, uint16_t device_id) {
    // Common Intel devices
    if (vendor_id == 0x8086) {
        switch (device_id) {
            case 0x1237: return "82441FX PCI & Memory Controller";
            case 0x7000: return "82371SB PIIX3 ISA";
            case 0x7010: return "82371SB PIIX3 IDE";
            case 0x7020: return "82371SB PIIX3 USB";
            case 0x7113: return "82371AB/EB/MB PIIX4 ACPI";
            case 0x100E: return "82540EM Gigabit Ethernet";
            case 0x100F: return "82545EM Gigabit Ethernet";
            case 0x10D3: return "82574L Gigabit Ethernet";
            case 0x2415: return "82801AA AC'97 Audio";
            case 0x2668: return "82801FB HD Audio";
            default: return "Intel Device";
        }
    }
    
    // Common AMD devices
    if (vendor_id == 0x1022) {
        switch (device_id) {
            case 0x2000: return "79c970 PCnet32";
            case 0x2020: return "79c973 PCnet-FAST III";
            default: return "AMD Device";
        }
    }
    
    // VirtIO devices
    if (vendor_id == 0x1AF4) {
        switch (device_id) {
            case 0x1000: return "VirtIO Network";
            case 0x1001: return "VirtIO Block";
            case 0x1002: return "VirtIO Balloon";
            case 0x1003: return "VirtIO Console";
            case 0x1004: return "VirtIO SCSI";
            case 0x1005: return "VirtIO RNG";
            case 0x1009: return "VirtIO 9P Transport";
            default: return "VirtIO Device";
        }
    }
    
    return "Unknown Device";
}

const char* pci_get_class_name(uint8_t class_code) {
    switch (class_code) {
        case 0x00: return "Unclassified";
        case 0x01: return "Mass Storage Controller";
        case 0x02: return "Network Controller";
        case 0x03: return "Display Controller";
        case 0x04: return "Multimedia Device";
        case 0x05: return "Memory Controller";
        case 0x06: return "Bridge Device";
        case 0x07: return "Simple Communication Controller";
        case 0x08: return "Base System Peripheral";
        case 0x09: return "Input Device";
        case 0x0A: return "Docking Station";
        case 0x0B: return "Processor";
        case 0x0C: return "Serial Bus Controller";
        case 0x0D: return "Wireless Controller";
        case 0x0E: return "Intelligent Controller";
        case 0x0F: return "Satellite Communication Controller";
        case 0x10: return "Encryption/Decryption Controller";
        case 0x11: return "Data Acquisition Controller";
        case 0xFF: return "Unassigned Class";
        default: return "Unknown Class";
    }
}

const char* pci_get_subclass_name(uint8_t class_code, uint8_t subclass) {
    switch (class_code) {
        case 0x01: // Mass Storage
            switch (subclass) {
                case 0x00: return "SCSI Bus Controller";
                case 0x01: return "IDE Controller";
                case 0x02: return "Floppy Disk Controller";
                case 0x03: return "IPI Bus Controller";
                case 0x04: return "RAID Controller";
                case 0x05: return "ATA Controller";
                case 0x06: return "SATA Controller";
                case 0x07: return "Serial Attached SCSI";
                case 0x08: return "NVM Controller";
                default: return "Mass Storage Controller";
            }
            
        case 0x02: // Network
            switch (subclass) {
                case 0x00: return "Ethernet Controller";
                case 0x01: return "Token Ring Controller";
                case 0x02: return "FDDI Controller";
                case 0x03: return "ATM Controller";
                case 0x04: return "ISDN Controller";
                case 0x05: return "WorldFip Controller";
                case 0x06: return "PICMG 2.14";
                case 0x07: return "InfiniBand Controller";
                case 0x08: return "Host Fabric Controller";
                default: return "Network Controller";
            }
            
        case 0x03: // Display
            switch (subclass) {
                case 0x00: return "VGA Controller";
                case 0x01: return "XGA Controller";
                case 0x02: return "3D Controller";
                default: return "Display Controller";
            }
            
        case 0x04: // Multimedia
            switch (subclass) {
                case 0x00: return "Video Device";
                case 0x01: return "Audio Device";
                case 0x02: return "Computer Telephony Device";
                case 0x03: return "HD Audio Device";
                default: return "Multimedia Device";
            }
            
        case 0x06: // Bridge
            switch (subclass) {
                case 0x00: return "Host Bridge";
                case 0x01: return "ISA Bridge";
                case 0x02: return "EISA Bridge";
                case 0x03: return "MCA Bridge";
                case 0x04: return "PCI-to-PCI Bridge";
                case 0x05: return "PCMCIA Bridge";
                case 0x06: return "NuBus Bridge";
                case 0x07: return "CardBus Bridge";
                case 0x08: return "RACEway Bridge";
                case 0x09: return "Semi-Transparent PCI-to-PCI Bridge";
                case 0x0A: return "InfiniBand-to-PCI Host Bridge";
                default: return "Bridge Device";
            }
            
        case 0x0C: // Serial Bus
            switch (subclass) {
                case 0x00: return "IEEE 1394 Controller";
                case 0x01: return "ACCESS Bus Controller";
                case 0x02: return "SSA Controller";
                case 0x03: return "USB Controller";
                case 0x04: return "Fibre Channel Controller";
                case 0x05: return "SMBus Controller";
                case 0x06: return "InfiniBand Controller";
                case 0x07: return "IPMI Interface";
                case 0x08: return "SERCOS Interface";
                case 0x09: return "CANbus Controller";
                default: return "Serial Bus Controller";
            }
            
        default:
            return "Unknown Subclass";
    }
}

void pci_print_device_info(pci_device_t* dev) {
    kprintf("PCI Device %02x:%02x.%x:\n", dev->bus, dev->device, dev->function);
    kprintf("  Vendor: 0x%04x (%s)\n", dev->vendor_id, pci_get_vendor_name(dev->vendor_id));
    kprintf("  Device: 0x%04x (%s)\n", dev->device_id, pci_get_device_name(dev->vendor_id, dev->device_id));
    kprintf("  Class:  0x%02x (%s)\n", dev->class_code, pci_get_class_name(dev->class_code));
    kprintf("  Subclass: 0x%02x (%s)\n", dev->subclass, pci_get_subclass_name(dev->class_code, dev->subclass));
    kprintf("  Prog IF: 0x%02x\n", dev->prog_if);
    kprintf("  Rev ID:  0x%02x\n", dev->revision_id);
    
    if ((dev->header_type & PCI_HEADER_TYPE_MASK) == PCI_HEADER_TYPE_NORMAL) {
        kprintf("  Subsystem: %04x:%04x\n", dev->subsystem_vendor_id, dev->subsystem_id);
        
        // Print BARs
        for (int i = 0; i < 6; i++) {
            if (dev->bar[i] != 0) {
                uint32_t addr = pci_get_bar_address(dev, i);
                uint32_t size = pci_get_bar_size(dev, i);
                
                if (pci_bar_is_io(dev, i)) {
                    kprintf("  BAR%d: I/O at 0x%04x (size=%d)\n", i, addr, size);
                } else {
                    kprintf("  BAR%d: Memory at 0x%08x (size=%d%s%s)\n", 
                           i, addr, size,
                           pci_bar_is_64bit(dev, i) ? ", 64-bit" : ", 32-bit",
                           pci_bar_is_prefetchable(dev, i) ? ", prefetchable" : "");
                    
                    // Skip next BAR if this is 64-bit
                    if (pci_bar_is_64bit(dev, i)) i++;
                }
            }
        }
        
        if (dev->interrupt_pin != 0) {
            kprintf("  IRQ: pin %c routed to line %d\n", 
                   'A' + dev->interrupt_pin - 1, dev->interrupt_line);
        }
    }
}

void pci_print_all_devices(void) {
    kprintf("\n=== PCI Device Listing ===\n");
    pci_device_t* dev = pci_devices;
    int count = 0;
    
    while (dev) {
        pci_print_device_info(dev);
        kprintf("\n");
        count++;
        dev = dev->next;
    }
    
    kprintf("Total devices: %d\n", count);
}