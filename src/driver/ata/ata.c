#include <intel86.h>
#include <pci/pci.h>
#include <driver/ata/ata.h>
#include <memory/memory.h>
#include <string.h>
#include <print.h>
#include <io.h>

// Global ATA Controller
static AtaController g_ata_controller = {0};

// Standard IDE/ATA I/O Ports
#define ATA_PRIMARY_IO     0x1F0
#define ATA_PRIMARY_CTRL   0x3F6
#define ATA_SECONDARY_IO   0x170
#define ATA_SECONDARY_CTRL 0x376

// Helper macros for I/O operations
#define ata_inb(port)       inb(port)
#define ata_inw(port)       inw(port)
#define ata_outb(port, val) outb(port, val)
#define ata_outw(port, val) outw(port, val)

// Private helper functions
static void ata_io_wait(uint8_t channel);
static void ata_select_drive(uint8_t channel, uint8_t drive);
static bool ata_identify_device(uint8_t channel, uint8_t drive, uint16_t* buffer);
static void ata_fix_string(char* str, size_t len);
static void ata_scan_pci_controllers(void);
static bool ata_is_ide_controller(pci_device_t* device);

// Initialize ATA controller
void ata_init(void) {
    printf("[ATA] Initializing ATA subsystem...\n");
    
    // Initialize channels with standard I/O ports (legacy fallback)
    g_ata_controller.channels[0].base = ATA_PRIMARY_IO;
    g_ata_controller.channels[0].ctrl = ATA_PRIMARY_CTRL;
    g_ata_controller.channels[0].bmide = 0; // Will be set if PCI device found
    g_ata_controller.channels[0].nien = 0;
    
    g_ata_controller.channels[1].base = ATA_SECONDARY_IO;
    g_ata_controller.channels[1].ctrl = ATA_SECONDARY_CTRL;
    g_ata_controller.channels[1].bmide = 0;
    g_ata_controller.channels[1].nien = 0;
    
    g_ata_controller.device_count = 0;
    g_ata_controller.irq_invoked = false;
    
    // Scan PCI bus for ATA/IDE controllers
    printf("[ATA] Scanning PCI bus for ATA/IDE controllers...\n");
    ata_scan_pci_controllers();
    
    // Disable interrupts during detection
    ata_outb(g_ata_controller.channels[0].ctrl + ATA_REG_CONTROL, 2);
    ata_outb(g_ata_controller.channels[1].ctrl + ATA_REG_CONTROL, 2);
    
    // Detect all devices
    ata_detect_devices();
    
    printf("[ATA] ATA subsystem initialization complete.\n");
}

// Detect all ATA devices
void ata_detect_devices(void) {
    int device_index = 0;
    
    printf("[ATA] Starting device detection...\n");
    
    for (uint8_t channel = 0; channel < 2; channel++) {
        printf("[ATA] Scanning channel %d...\n", channel);
        for (uint8_t drive = 0; drive < 2; drive++) {
            printf("[ATA] Checking channel %d, drive %d...\n", channel, drive);
            if (ata_device_init(channel, drive)) {
                device_index++;
                printf("[ATA] Device found on channel %d, drive %d\n", channel, drive);
            } else {
                printf("[ATA] No device on channel %d, drive %d\n", channel, drive);
            }
        }
    }
    
    printf("[ATA] Detected %d device(s)\n", g_ata_controller.device_count);
}

// Initialize a specific ATA device
bool __attribute__((optimize("O0"))) ata_device_init(uint8_t channel, uint8_t drive) {
    uint8_t device_index = channel * 2 + drive;
    AtaDevice* device = &g_ata_controller.devices[device_index];
    
    printf("[ATA] Initializing device: channel=%d, drive=%d\n", channel, drive);
    
    // Reset device structure
    memset(device, 0, sizeof(AtaDevice));
    
    device->channel = channel;
    device->drive = drive;
    device->channel_info = &g_ata_controller.channels[channel];
    device->exists = false;
    
    // Select drive
    printf("[ATA] Selecting drive...\n");
    ata_select_drive(channel, drive);
    ata_io_wait(channel);
    
    // Send IDENTIFY command
    printf("[ATA] Sending IDENTIFY command...\n");
    ata_outb(g_ata_controller.channels[channel].base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    ata_io_wait(channel);
    
    // Check if device exists
    uint8_t status = ata_inb(g_ata_controller.channels[channel].base + ATA_REG_STATUS);
    printf("[ATA] Initial status: 0x%02X\n", status);
    if (status == 0 || status == 0xFF) {
        printf("[ATA] No device detected (status 0x%02X)\n", status);
        return false; // No device
    }
    
    // Wait for device to be ready with timeout
    uint32_t timeout = 1000;
    while (timeout > 0) {
        status = ata_inb(g_ata_controller.channels[channel].base + ATA_REG_STATUS);
        if (status & ATA_SR_ERR) {
            // Not ATA, might be ATAPI
            uint8_t cl = ata_inb(g_ata_controller.channels[channel].base + ATA_REG_LBA1);
            uint8_t ch = ata_inb(g_ata_controller.channels[channel].base + ATA_REG_LBA2);
            
            if (cl == 0x14 && ch == 0xEB) {
                device->type = ATA_TYPE_PATAPI;
            } else if (cl == 0x69 && ch == 0x96) {
                device->type = ATA_TYPE_SATAPI;
            } else {
                return false; // Unknown device
            }
            
            // Send IDENTIFY PACKET command for ATAPI
            ata_outb(g_ata_controller.channels[channel].base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
            ata_io_wait(channel);
            break;
        }
        
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) {
            break;
        }
        
        timeout--;
        if (timeout == 0) {
            printf("[ATA] Timeout waiting for device response on channel %d drive %d\n", channel, drive);
            return false;
        }
        
        // Small delay
        ata_io_wait(channel);
    }
    
    // Read identification data
    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = ata_inw(g_ata_controller.channels[channel].base + ATA_REG_DATA);
    }
    
    // Parse identification data
    device->exists = true;
    device->signature = identify_data[0];
    
    // Determine device type if not ATAPI
    if (device->type != ATA_TYPE_PATAPI && device->type != ATA_TYPE_SATAPI) {
        if (identify_data[0] == 0x0040) {
            device->type = ATA_TYPE_PATA;
        } else {
            device->type = ATA_TYPE_SATA;
        }
    }
    
    // Copy model, serial, and firmware strings
    for (int i = 0; i < 20; i++) {
        uint16_t word = identify_data[27 + i];
        device->model[i*2] = (word >> 8) & 0xFF;
        device->model[i*2 + 1] = word & 0xFF;
    }
    device->model[40] = '\0';
    ata_fix_string(device->model, 40);
    
    for (int i = 0; i < 10; i++) {
        uint16_t word = identify_data[10 + i];
        device->serial[i*2] = (word >> 8) & 0xFF;
        device->serial[i*2 + 1] = word & 0xFF;
    }
    device->serial[20] = '\0';
    ata_fix_string(device->serial, 20);
    
    for (int i = 0; i < 4; i++) {
        uint16_t word = identify_data[23 + i];
        device->firmware[i*2] = (word >> 8) & 0xFF;
        device->firmware[i*2 + 1] = word & 0xFF;
    }
    device->firmware[8] = '\0';
    ata_fix_string(device->firmware, 8);
    
    // Determine addressing mode and size
    if (identify_data[83] & (1 << 10)) {
        // LBA48 supported
        device->addressing_mode = ATA_MODE_LBA48;
        device->size = ((uint64_t)identify_data[103] << 48) |
                      ((uint64_t)identify_data[102] << 32) |
                      ((uint64_t)identify_data[101] << 16) |
                      ((uint64_t)identify_data[100]);
    } else if (identify_data[49] & (1 << 9)) {
        // LBA28 supported
        device->addressing_mode = ATA_MODE_LBA28;
        device->size = ((uint32_t)identify_data[61] << 16) | identify_data[60];
    } else {
        // CHS only
        device->addressing_mode = ATA_MODE_CHS;
        uint16_t cylinders = identify_data[1];
        uint16_t heads = identify_data[3];
        uint16_t sectors = identify_data[6];
        device->size = cylinders * heads * sectors;
    }
    
    // Set sector size (usually 512 bytes)
    device->sector_size = 512;
    if (identify_data[106] & (1 << 12)) {
        // Logical sector size is not 512 bytes
        uint32_t logical_sector_size = ((uint32_t)identify_data[118] << 16) | identify_data[117];
        if (logical_sector_size > 0) {
            device->sector_size = logical_sector_size * 2; // Words to bytes
        }
    }
    
    device->total_size_bytes = device->size * device->sector_size;
    
    // Check DMA support
    device->dma_supported = (identify_data[49] & (1 << 8)) != 0;
    if (device->dma_supported) {
        // Check UDMA modes
        uint16_t udma_modes = identify_data[88];
        for (int i = 7; i >= 0; i--) {
            if (udma_modes & (1 << (i + 8))) {
                device->udma_mode = i;
                break;
            }
        }
    }
    
    // Check cache support
    device->write_cache_enabled = (identify_data[85] & (1 << 5)) != 0;
    device->read_cache_enabled = (identify_data[85] & (1 << 6)) != 0;
    
    g_ata_controller.device_count++;
    
    return true;
}

// Create ATA device from PCI device
AtaDevice* create_ata_device(pci_device_t* pciDevice) {
    if (!pciDevice) {
        return NULL;
    }
    
    // Check if it's an IDE/ATA controller
    if (!ata_is_ide_controller(pciDevice)) {
        return NULL;
    }
    
    printf("[ATA] Creating device from PCI controller %04X:%04X\n", 
           pciDevice->vendor_id, pciDevice->device_id);
    
    // Configure the PCI controller
    ata_configure_pci_controller(pciDevice);
    
    // Initialize ATA subsystem if not already done
    static bool ata_initialized = false;
    if (!ata_initialized) {
        ata_init();
        ata_initialized = true;
    }
    
    // Return first available device
    for (int i = 0; i < 4; i++) {
        if (g_ata_controller.devices[i].exists) {
            g_ata_controller.devices[i].pci_device = pciDevice;
            return &g_ata_controller.devices[i];
        }
    }
    
    return NULL;
}

// Read sectors using PIO mode
bool ata_read_sectors(AtaDevice* device, uint64_t lba, uint16_t count, void* buffer) {
    if (!device || !device->exists || !buffer || count == 0) {
        return false;
    }
    
    uint8_t channel = device->channel;
    uint8_t drive = device->drive;
    uint16_t base = g_ata_controller.channels[channel].base;
    uint8_t* data = (uint8_t*)buffer;
    
    // Wait for device to be ready
    ata_wait_busy(channel);
    
    // Select drive and addressing mode
    uint8_t drive_select = 0xE0 | (drive << 4);
    
    if (device->addressing_mode == ATA_MODE_LBA48 && (lba >= 0x10000000 || count > 256)) {
        // LBA48 mode
        drive_select |= 0x40;
        ata_outb(base + ATA_REG_HDDEVSEL, drive_select);
        ata_io_wait(channel);
        
        // Send high order bytes
        ata_outb(base + ATA_REG_SECCOUNT1, (count >> 8) & 0xFF);
        ata_outb(base + ATA_REG_LBA3, (lba >> 24) & 0xFF);
        ata_outb(base + ATA_REG_LBA4, (lba >> 32) & 0xFF);
        ata_outb(base + ATA_REG_LBA5, (lba >> 40) & 0xFF);
        
        // Send low order bytes
        ata_outb(base + ATA_REG_SECCOUNT0, count & 0xFF);
        ata_outb(base + ATA_REG_LBA0, lba & 0xFF);
        ata_outb(base + ATA_REG_LBA1, (lba >> 8) & 0xFF);
        ata_outb(base + ATA_REG_LBA2, (lba >> 16) & 0xFF);
        
        // Send command
        ata_outb(base + ATA_REG_COMMAND, ATA_CMD_READ_PIO_EXT);
    } else if (device->addressing_mode != ATA_MODE_CHS) {
        // LBA28 mode
        drive_select |= 0x40 | ((lba >> 24) & 0x0F);
        ata_outb(base + ATA_REG_HDDEVSEL, drive_select);
        ata_io_wait(channel);
        
        ata_outb(base + ATA_REG_FEATURES, 0);
        ata_outb(base + ATA_REG_SECCOUNT0, count & 0xFF);
        ata_outb(base + ATA_REG_LBA0, lba & 0xFF);
        ata_outb(base + ATA_REG_LBA1, (lba >> 8) & 0xFF);
        ata_outb(base + ATA_REG_LBA2, (lba >> 16) & 0xFF);
        
        // Send command
        ata_outb(base + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
    } else {
        // CHS mode (legacy)
        uint32_t cylinder = lba / (16 * 63);
        uint32_t head = (lba / 63) % 16;
        uint32_t sector = (lba % 63) + 1;
        
        drive_select |= head & 0x0F;
        ata_outb(base + ATA_REG_HDDEVSEL, drive_select);
        ata_io_wait(channel);
        
        ata_outb(base + ATA_REG_FEATURES, 0);
        ata_outb(base + ATA_REG_SECCOUNT0, count);
        ata_outb(base + ATA_REG_LBA0, sector);
        ata_outb(base + ATA_REG_LBA1, cylinder & 0xFF);
        ata_outb(base + ATA_REG_LBA2, (cylinder >> 8) & 0xFF);
        
        // Send command
        ata_outb(base + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
    }
    
    // Read data
    for (uint16_t i = 0; i < count; i++) {
        // Wait for data
        if (!ata_polling(channel, true)) {
            return false;
        }
        
        // Read sector
        for (int j = 0; j < 256; j++) {
            uint16_t word = ata_inw(base + ATA_REG_DATA);
            data[i * 512 + j * 2] = word & 0xFF;
            data[i * 512 + j * 2 + 1] = (word >> 8) & 0xFF;
        }
    }
    
    return true;
}

// Write sectors using PIO mode
bool ata_write_sectors(AtaDevice* device, uint64_t lba, uint16_t count, const void* buffer) {
    if (!device || !device->exists || !buffer || count == 0) {
        return false;
    }
    
    uint8_t channel = device->channel;
    uint8_t drive = device->drive;
    uint16_t base = g_ata_controller.channels[channel].base;
    const uint8_t* data = (const uint8_t*)buffer;
    
    // Wait for device to be ready
    ata_wait_busy(channel);
    
    // Select drive and addressing mode
    uint8_t drive_select = 0xE0 | (drive << 4);
    
    if (device->addressing_mode == ATA_MODE_LBA48 && (lba >= 0x10000000 || count > 256)) {
        // LBA48 mode
        drive_select |= 0x40;
        ata_outb(base + ATA_REG_HDDEVSEL, drive_select);
        ata_io_wait(channel);
        
        // Send high order bytes
        ata_outb(base + ATA_REG_SECCOUNT1, (count >> 8) & 0xFF);
        ata_outb(base + ATA_REG_LBA3, (lba >> 24) & 0xFF);
        ata_outb(base + ATA_REG_LBA4, (lba >> 32) & 0xFF);
        ata_outb(base + ATA_REG_LBA5, (lba >> 40) & 0xFF);
        
        // Send low order bytes
        ata_outb(base + ATA_REG_SECCOUNT0, count & 0xFF);
        ata_outb(base + ATA_REG_LBA0, lba & 0xFF);
        ata_outb(base + ATA_REG_LBA1, (lba >> 8) & 0xFF);
        ata_outb(base + ATA_REG_LBA2, (lba >> 16) & 0xFF);
        
        // Send command
        ata_outb(base + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO_EXT);
    } else if (device->addressing_mode != ATA_MODE_CHS) {
        // LBA28 mode
        drive_select |= 0x40 | ((lba >> 24) & 0x0F);
        ata_outb(base + ATA_REG_HDDEVSEL, drive_select);
        ata_io_wait(channel);
        
        ata_outb(base + ATA_REG_FEATURES, 0);
        ata_outb(base + ATA_REG_SECCOUNT0, count & 0xFF);
        ata_outb(base + ATA_REG_LBA0, lba & 0xFF);
        ata_outb(base + ATA_REG_LBA1, (lba >> 8) & 0xFF);
        ata_outb(base + ATA_REG_LBA2, (lba >> 16) & 0xFF);
        
        // Send command
        ata_outb(base + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
    } else {
        // CHS mode (legacy)
        uint32_t cylinder = lba / (16 * 63);
        uint32_t head = (lba / 63) % 16;
        uint32_t sector = (lba % 63) + 1;
        
        drive_select |= head & 0x0F;
        ata_outb(base + ATA_REG_HDDEVSEL, drive_select);
        ata_io_wait(channel);
        
        ata_outb(base + ATA_REG_FEATURES, 0);
        ata_outb(base + ATA_REG_SECCOUNT0, count);
        ata_outb(base + ATA_REG_LBA0, sector);
        ata_outb(base + ATA_REG_LBA1, cylinder & 0xFF);
        ata_outb(base + ATA_REG_LBA2, (cylinder >> 8) & 0xFF);
        
        // Send command
        ata_outb(base + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
    }
    
    // Write data
    for (uint16_t i = 0; i < count; i++) {
        // Wait for device to be ready for data
        if (!ata_polling(channel, false)) {
            return false;
        }
        
        // Write sector
        for (int j = 0; j < 256; j++) {
            uint16_t word = data[i * 512 + j * 2] | (data[i * 512 + j * 2 + 1] << 8);
            ata_outw(base + ATA_REG_DATA, word);
        }
        
        // Flush cache after each sector
        ata_outb(base + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
        ata_polling(channel, false);
    }
    
    return true;
}

// Read single sector
bool ata_read_sector(AtaDevice* device, uint64_t lba, void* buffer) {
    return ata_read_sectors(device, lba, 1, buffer);
}

// Write single sector
bool ata_write_sector(AtaDevice* device, uint64_t lba, const void* buffer) {
    return ata_write_sectors(device, lba, 1, buffer);
}

// Get disk size in sectors
uint64_t ata_get_disk_size(AtaDevice* device) {
    if (!device || !device->exists) {
        return 0;
    }
    return device->size;
}

// Get sector size in bytes
uint32_t ata_get_sector_size(AtaDevice* device) {
    if (!device || !device->exists) {
        return 0;
    }
    return device->sector_size;
}

// Get total size in bytes
uint64_t ata_get_total_size_bytes(AtaDevice* device) {
    if (!device || !device->exists) {
        return 0;
    }
    return device->total_size_bytes;
}

// Get device model string
const char* ata_get_model(AtaDevice* device) {
    if (!device || !device->exists) {
        return "Unknown";
    }
    return device->model;
}

// Get device serial number
const char* ata_get_serial(AtaDevice* device) {
    if (!device || !device->exists) {
        return "Unknown";
    }
    return device->serial;
}

// Get device firmware version
const char* ata_get_firmware(AtaDevice* device) {
    if (!device || !device->exists) {
        return "Unknown";
    }
    return device->firmware;
}

// Flush device cache
bool ata_flush_cache(AtaDevice* device) {
    if (!device || !device->exists) {
        return false;
    }
    
    uint8_t channel = device->channel;
    uint16_t base = g_ata_controller.channels[channel].base;
    
    ata_wait_busy(channel);
    
    if (device->addressing_mode == ATA_MODE_LBA48) {
        ata_outb(base + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH_EXT);
    } else {
        ata_outb(base + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
    }
    
    return ata_polling(channel, false);
}

// Wait for device to not be busy
void ata_wait_busy(uint8_t channel) {
    uint16_t base = g_ata_controller.channels[channel].base;
    while (ata_inb(base + ATA_REG_STATUS) & ATA_SR_BSY) {
        // Busy wait
    }
}

// Wait for data request ready
void ata_wait_drq(uint8_t channel) {
    uint16_t base = g_ata_controller.channels[channel].base;
    while (!(ata_inb(base + ATA_REG_STATUS) & ATA_SR_DRQ)) {
        // Wait for DRQ
    }
}

// Wait for device ready with timeout
bool ata_wait_ready(uint8_t channel, uint32_t timeout_ms) {
    uint16_t base = g_ata_controller.channels[channel].base;
    uint32_t count = 0;
    
    while (count < timeout_ms * 1000) {
        uint8_t status = ata_inb(base + ATA_REG_STATUS);
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRDY)) {
            return true;
        }
        // Simple delay (approximately 1 microsecond)
        for (volatile int i = 0; i < 10; i++);
        count++;
    }
    
    return false;
}

// Read status register
uint8_t ata_read_status(uint8_t channel) {
    return ata_inb(g_ata_controller.channels[channel].base + ATA_REG_STATUS);
}

// 400ns delay by reading alternate status register
void ata_400ns_delay(uint8_t channel) {
    for (int i = 0; i < 4; i++) {
        ata_inb(g_ata_controller.channels[channel].ctrl + ATA_REG_ALTSTATUS);
    }
}

// ATA polling with optional error checking
bool ata_polling(uint8_t channel, bool advanced_check) {
    uint16_t base = g_ata_controller.channels[channel].base;
    
    // Wait 400ns
    ata_400ns_delay(channel);
    
    // Wait for BSY to clear
    uint8_t status;
    uint32_t timeout = 500000; // Timeout counter
    while (--timeout) {
        status = ata_inb(base + ATA_REG_STATUS);
        if (!(status & ATA_SR_BSY)) {
            break;
        }
    }
    
    if (timeout == 0) {
        return false; // Timeout
    }
    
    if (advanced_check) {
        status = ata_inb(base + ATA_REG_STATUS);
        
        // Check for errors
        if (status & ATA_SR_ERR) {
            return false;
        }
        
        // Check for drive fault
        if (status & ATA_SR_DF) {
            return false;
        }
        
        // Check for DRQ
        if (!(status & ATA_SR_DRQ)) {
            return false;
        }
    }
    
    return true;
}

// Get error register value
uint8_t ata_get_error(uint8_t channel) {
    return ata_inb(g_ata_controller.channels[channel].base + ATA_REG_ERROR);
}

// Get error string from error code
const char* ata_get_error_string(uint8_t error) {
    if (error & ATA_ER_BBK) return "Bad Block";
    if (error & ATA_ER_UNC) return "Uncorrectable Data";
    if (error & ATA_ER_MC) return "Media Changed";
    if (error & ATA_ER_IDNF) return "ID Not Found";
    if (error & ATA_ER_MCR) return "Media Change Request";
    if (error & ATA_ER_ABRT) return "Command Aborted";
    if (error & ATA_ER_TK0NF) return "Track 0 Not Found";
    if (error & ATA_ER_AMNF) return "Address Mark Not Found";
    return "Unknown Error";
}

// Print error information
void ata_print_error(AtaDevice* device) {
    if (!device || !device->exists) {
        return;
    }
    
    uint8_t error = ata_get_error(device->channel);
    printf("[ATA] Error on %s %s: %s (0x%02X)\n",
           device->channel ? "Secondary" : "Primary",
           device->drive ? "Slave" : "Master",
           ata_get_error_string(error),
           error);
}

// Soft reset a channel
void ata_soft_reset(uint8_t channel) {
    uint16_t ctrl = g_ata_controller.channels[channel].ctrl;
    
    // Set SRST bit
    ata_outb(ctrl + ATA_REG_CONTROL, 0x04);
    ata_400ns_delay(channel);
    
    // Clear SRST bit
    ata_outb(ctrl + ATA_REG_CONTROL, 0x00);
    ata_400ns_delay(channel);
    
    // Wait for devices to be ready
    ata_wait_ready(channel, 5000);
}

// Get device by index
AtaDevice* ata_get_device(uint8_t index) {
    if (index >= 4) {
        return NULL;
    }
    
    if (g_ata_controller.devices[index].exists) {
        return &g_ata_controller.devices[index];
    }
    
    return NULL;
}

// Get device count
uint8_t ata_get_device_count(void) {
    return g_ata_controller.device_count;
}

// Get device by channel and drive
AtaDevice* ata_get_device_by_path(uint8_t channel, uint8_t drive) {
    if (channel > 1 || drive > 1) {
        return NULL;
    }
    
    uint8_t index = channel * 2 + drive;
    if (g_ata_controller.devices[index].exists) {
        return &g_ata_controller.devices[index];
    }
    
    return NULL;
}

// Print device information
void ata_print_device_info(AtaDevice* device) {
    if (!device || !device->exists) {
        printf("[ATA] Invalid or non-existent device\n");
        return;
    }
    
    const char* type_str = "Unknown";
    switch (device->type) {
        case ATA_TYPE_PATA: type_str = "PATA"; break;
        case ATA_TYPE_SATA: type_str = "SATA"; break;
        case ATA_TYPE_PATAPI: type_str = "PATAPI"; break;
        case ATA_TYPE_SATAPI: type_str = "SATAPI"; break;
    }
    
    const char* addr_mode = "Unknown";
    switch (device->addressing_mode) {
        case ATA_MODE_CHS: addr_mode = "CHS"; break;
        case ATA_MODE_LBA28: addr_mode = "LBA28"; break;
        case ATA_MODE_LBA48: addr_mode = "LBA48"; break;
    }
    
    printf("[ATA] %s %s:\n", 
           device->channel ? "Secondary" : "Primary",
           device->drive ? "Slave" : "Master");
    printf("  Type: %s\n", type_str);
    printf("  Model: %s\n", device->model);
    printf("  Serial: %s\n", device->serial);
    printf("  Firmware: %s\n", device->firmware);
    printf("  Addressing: %s\n", addr_mode);
    printf("  Sectors: %llu\n", device->size);
    printf("  Sector Size: %u bytes\n", device->sector_size);
    printf("  Total Size: %llu MB\n", (size_t)device->total_size_bytes / (1024 * 1024));
    printf("  DMA: %s\n", device->dma_supported ? "Supported" : "Not Supported");
    if (device->dma_supported) {
        printf("  UDMA Mode: %u\n", device->udma_mode);
    }
}

// Print all devices
void ata_print_all_devices(void) {
    printf("[ATA] Device List:\n");
    printf("================\n");
    
    if (g_ata_controller.device_count == 0) {
        printf("No ATA devices found\n");
        return;
    }
    
    for (int i = 0; i < 4; i++) {
        if (g_ata_controller.devices[i].exists) {
            ata_print_device_info(&g_ata_controller.devices[i]);
            printf("\n");
        }
    }
}

// Helper: I/O wait
static void ata_io_wait(uint8_t channel) {
    ata_400ns_delay(channel);
}

// Helper: Select drive
static void ata_select_drive(uint8_t channel, uint8_t drive) {
    uint16_t base = g_ata_controller.channels[channel].base;
    ata_outb(base + ATA_REG_HDDEVSEL, 0xA0 | (drive << 4));
    ata_io_wait(channel);
}

// Helper: Fix ATA string (swap bytes and trim)
static void ata_fix_string(char* str, size_t len) {
    // Swap byte pairs (ATA strings are weird)
    for (size_t i = 0; i < len; i += 2) {
        char tmp = str[i];
        str[i] = str[i + 1];
        str[i + 1] = tmp;
    }
    
    // Trim trailing spaces
    for (int i = len - 1; i >= 0; i--) {
        if (str[i] == ' ' || str[i] == '\0') {
            str[i] = '\0';
        } else {
            break;
        }
    }
}

// ATAPI: Read sectors (for CD/DVD)
bool atapi_read_sectors(AtaDevice* device, uint32_t lba, uint16_t count, void* buffer) {
    if (!device || !device->exists || !buffer) {
        return false;
    }
    
    if (device->type != ATA_TYPE_PATAPI && device->type != ATA_TYPE_SATAPI) {
        return false; // Not an ATAPI device
    }
    
    uint8_t channel = device->channel;
    uint8_t drive = device->drive;
    uint16_t base = g_ata_controller.channels[channel].base;
    uint8_t* data = (uint8_t*)buffer;
    
    // Select drive
    ata_select_drive(channel, drive);
    ata_400ns_delay(channel);
    
    // Set PIO mode
    ata_outb(base + ATA_REG_FEATURES, 0);
    
    // Set size
    ata_outb(base + ATA_REG_LBA1, (2048 * count) & 0xFF);
    ata_outb(base + ATA_REG_LBA2, ((2048 * count) >> 8) & 0xFF);
    
    // Send PACKET command
    ata_outb(base + ATA_REG_COMMAND, ATA_CMD_PACKET);
    
    // Wait for device
    if (!ata_polling(channel, true)) {
        return false;
    }
    
    // Send ATAPI packet (12 bytes)
    uint8_t packet[12] = {0};
    packet[0] = ATAPI_CMD_READ;
    packet[2] = (lba >> 24) & 0xFF;
    packet[3] = (lba >> 16) & 0xFF;
    packet[4] = (lba >> 8) & 0xFF;
    packet[5] = lba & 0xFF;
    packet[9] = count;
    
    for (int i = 0; i < 6; i++) {
        ata_outw(base + ATA_REG_DATA, ((uint16_t*)packet)[i]);
    }
    
    // Read data
    for (uint16_t i = 0; i < count; i++) {
        // Wait for data
        if (!ata_polling(channel, true)) {
            return false;
        }
        
        // Read sector (2048 bytes for ATAPI)
        for (int j = 0; j < 1024; j++) {
            uint16_t word = ata_inw(base + ATA_REG_DATA);
            data[i * 2048 + j * 2] = word & 0xFF;
            data[i * 2048 + j * 2 + 1] = (word >> 8) & 0xFF;
        }
    }
    
    return true;
}

// ATAPI: Eject media
bool atapi_eject(AtaDevice* device) {
    if (!device || !device->exists) {
        return false;
    }
    
    if (device->type != ATA_TYPE_PATAPI && device->type != ATA_TYPE_SATAPI) {
        return false; // Not an ATAPI device
    }
    
    uint8_t channel = device->channel;
    uint8_t drive = device->drive;
    uint16_t base = g_ata_controller.channels[channel].base;
    
    // Select drive
    ata_select_drive(channel, drive);
    ata_400ns_delay(channel);
    
    // Send PACKET command
    ata_outb(base + ATA_REG_COMMAND, ATA_CMD_PACKET);
    
    // Wait for device
    if (!ata_polling(channel, true)) {
        return false;
    }
    
    // Send ATAPI EJECT packet
    uint8_t packet[12] = {0};
    packet[0] = ATAPI_CMD_EJECT;
    packet[4] = 0x02; // Eject
    
    for (int i = 0; i < 6; i++) {
        ata_outw(base + ATA_REG_DATA, ((uint16_t*)packet)[i]);
    }
    
    return ata_polling(channel, false);
}

// Scan PCI bus for ATA/IDE controllers
static void ata_scan_pci_controllers(void) {
    printf("[ATA] Scanning PCI devices for IDE/ATA controllers...\n");
    
    // Get all PCI devices
    pci_device_t* device = pci_get_all_devices();
    bool found_controller = false;
    
    while (device != NULL) {
        // Check if this is an IDE/ATA controller
        if (ata_is_ide_controller(device)) {
            printf("[ATA] Found IDE/ATA controller: %04X:%04X (Class: %02X, Subclass: %02X)\n",
                   device->vendor_id, device->device_id, 
                   device->class_code, device->subclass);
            
            // Configure this controller
            ata_configure_pci_controller(device);
            found_controller = true;
        }
        device = device->next;
    }
    
    if (!found_controller) {
        printf("[ATA] No PCI IDE/ATA controllers found, using legacy I/O ports\n");
    }
}

// Check if PCI device is an IDE/ATA controller
static bool ata_is_ide_controller(pci_device_t* device) {
    if (!device) {
        return false;
    }
    
    // Class 01h = Mass Storage Controller
    if (device->class_code != 0x01) {
        return false;
    }
    
    // Subclass codes for ATA/IDE:
    // 01h = IDE Controller
    // 05h = ATA Controller  
    // 06h = SATA Controller
    // 04h = RAID Controller (might also be ATA-based)
    return (device->subclass == 0x01 || // IDE
            device->subclass == 0x05 || // ATA  
            device->subclass == 0x06 || // SATA
            device->subclass == 0x04);  // RAID
}

// Configure PCI ATA/IDE controller
void ata_configure_pci_controller(pci_device_t* pci_device) {
    if (!pci_device) {
        return;
    }
    
    printf("[ATA] Configuring PCI controller %04X:%04X\n", 
           pci_device->vendor_id, pci_device->device_id);
    
    // Enable PCI device
    pci_enable_device(pci_device);
    pci_enable_bus_mastering(pci_device);
    
    // Read BAR registers for I/O ports
    uint32_t bar0 = pci_bar_read32(pci_device, 0, 0);
    uint32_t bar1 = pci_bar_read32(pci_device, 1, 0);
    uint32_t bar2 = pci_bar_read32(pci_device, 2, 0);
    uint32_t bar3 = pci_bar_read32(pci_device, 3, 0);
    uint32_t bar4 = pci_bar_read32(pci_device, 4, 0);
    
    printf("[ATA] BAR0: 0x%08X, BAR1: 0x%08X, BAR2: 0x%08X, BAR3: 0x%08X, BAR4: 0x%08X\n",
           bar0, bar1, bar2, bar3, bar4);
    
    // Update channel I/O ports if BARs are valid and in I/O space
    if (bar0 && (bar0 & 1)) { // BAR0: Primary channel command block
        uint16_t base = bar0 & 0xFFFC;
        if (base != 0x1F0) { // If not standard port, update it
            printf("[ATA] Primary channel command block: 0x%04X\n", base);
            g_ata_controller.channels[0].base = base;
        }
    }
    
    if (bar1 && (bar1 & 1)) { // BAR1: Primary channel control block
        uint16_t ctrl = bar1 & 0xFFFC;
        if (ctrl != 0x3F6) { // If not standard port, update it
            printf("[ATA] Primary channel control block: 0x%04X\n", ctrl);
            g_ata_controller.channels[0].ctrl = ctrl;
        }
    }
    
    if (bar2 && (bar2 & 1)) { // BAR2: Secondary channel command block
        uint16_t base = bar2 & 0xFFFC;
        if (base != 0x170) { // If not standard port, update it
            printf("[ATA] Secondary channel command block: 0x%04X\n", base);
            g_ata_controller.channels[1].base = base;
        }
    }
    
    if (bar3 && (bar3 & 1)) { // BAR3: Secondary channel control block
        uint16_t ctrl = bar3 & 0xFFFC;
        if (ctrl != 0x376) { // If not standard port, update it
            printf("[ATA] Secondary channel control block: 0x%04X\n", ctrl);
            g_ata_controller.channels[1].ctrl = ctrl;
        }
    }
    
    if (bar4 && (bar4 & 1)) { // BAR4: Bus Master IDE
        uint16_t bmide_base = bar4 & 0xFFFC;
        printf("[ATA] Bus Master IDE base: 0x%04X\n", bmide_base);
        g_ata_controller.channels[0].bmide = bmide_base + 0; // Primary channel
        g_ata_controller.channels[1].bmide = bmide_base + 8; // Secondary channel
    }
    
    // Set interrupt lines if available
    if (pci_device->interrupt_line != 0xFF) {
        printf("[ATA] IRQ line: %d\n", pci_device->interrupt_line);
        // TODO: Setup IRQ handlers when interrupt support is implemented
    }
    
    printf("[ATA] PCI controller configuration complete\n");
}