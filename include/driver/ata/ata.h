#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <pci/pci.h>

// ATA/IDE Register Offsets
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

// ATA Status Register Bits
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

// ATA Error Register Bits
#define ATA_ER_BBK     0x80    // Bad block
#define ATA_ER_UNC     0x40    // Uncorrectable data
#define ATA_ER_MC      0x20    // Media changed
#define ATA_ER_IDNF    0x10    // ID mark not found
#define ATA_ER_MCR     0x08    // Media change request
#define ATA_ER_ABRT    0x04    // Command aborted
#define ATA_ER_TK0NF   0x02    // Track 0 not found
#define ATA_ER_AMNF    0x01    // No address mark

// ATA Commands
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC
#define ATA_CMD_SET_FEATURES      0xEF

// ATAPI Commands
#define ATAPI_CMD_READ            0xA8
#define ATAPI_CMD_EJECT           0x1B

// ATA Drive Types
typedef enum {
    ATA_TYPE_UNKNOWN = 0,
    ATA_TYPE_PATA,
    ATA_TYPE_SATA,
    ATA_TYPE_PATAPI,
    ATA_TYPE_SATAPI
} AtaDriveType;

// ATA Addressing Modes
typedef enum {
    ATA_MODE_CHS = 0,
    ATA_MODE_LBA28,
    ATA_MODE_LBA48
} AtaAddressingMode;

// ATA Channel Structure
typedef struct {
    uint16_t base;      // I/O Base (Command Block)
    uint16_t ctrl;      // Control Base
    uint16_t bmide;     // Bus Master IDE
    uint8_t  nien;      // Interrupt disabled flag
} AtaChannel;

// ATA Device Structure
typedef struct {
    // Device identification
    bool exists;
    AtaDriveType type;
    AtaAddressingMode addressing_mode;
    uint8_t channel;    // 0 = Primary, 1 = Secondary
    uint8_t drive;      // 0 = Master, 1 = Slave
    
    // Device capabilities
    uint16_t signature;
    uint16_t capabilities;
    uint32_t command_sets;
    
    // Size information
    uint64_t size;      // Size in sectors
    uint32_t sector_size; // Usually 512 bytes
    uint64_t total_size_bytes;
    
    // Identification strings
    char model[41];
    char serial[21];
    char firmware[9];
    
    // Channel information
    AtaChannel* channel_info;
    
    // PCI device (if applicable)
    pci_device_t* pci_device;
    
    // For future abstraction layer
    void* driver_specific;
    void* fs_specific;
    
    // DMA support
    bool dma_supported;
    uint8_t udma_mode;
    
    // Cache
    bool write_cache_enabled;
    bool read_cache_enabled;
} AtaDevice;

// ATA Controller Structure (for managing multiple devices)
typedef struct {
    AtaChannel channels[2];  // Primary and Secondary
    AtaDevice devices[4];    // 4 devices max (2 per channel)
    uint8_t device_count;
    bool irq_invoked;
} AtaController;

// Function Prototypes

// Initialization
AtaDevice* create_ata_device(pci_device_t* pciDevice);
void ata_init(void);
void ata_detect_devices(void);
bool ata_device_init(uint8_t channel, uint8_t drive);
void ata_configure_pci_controller(pci_device_t* pci_device);

// Device Detection and Identification
bool ata_identify(AtaDevice* device);
bool ata_device_exists(uint8_t channel, uint8_t drive);
AtaDriveType ata_get_device_type(uint8_t channel, uint8_t drive);

// Basic I/O Operations
bool ata_read_sectors(AtaDevice* device, uint64_t lba, uint16_t count, void* buffer);
bool ata_write_sectors(AtaDevice* device, uint64_t lba, uint16_t count, const void* buffer);
bool ata_read_sector(AtaDevice* device, uint64_t lba, void* buffer);
bool ata_write_sector(AtaDevice* device, uint64_t lba, const void* buffer);

// Advanced I/O Operations (DMA)
bool ata_read_sectors_dma(AtaDevice* device, uint64_t lba, uint16_t count, void* buffer);
bool ata_write_sectors_dma(AtaDevice* device, uint64_t lba, uint16_t count, const void* buffer);

// Device Information
uint64_t ata_get_disk_size(AtaDevice* device);
uint32_t ata_get_sector_size(AtaDevice* device);
uint64_t ata_get_total_size_bytes(AtaDevice* device);
const char* ata_get_model(AtaDevice* device);
const char* ata_get_serial(AtaDevice* device);
const char* ata_get_firmware(AtaDevice* device);

// Device Control
bool ata_flush_cache(AtaDevice* device);
bool ata_set_features(AtaDevice* device, uint8_t feature);
void ata_soft_reset(uint8_t channel);
bool ata_device_select(AtaDevice* device);

// Utility Functions
void ata_wait_busy(uint8_t channel);
void ata_wait_drq(uint8_t channel);
bool ata_wait_ready(uint8_t channel, uint32_t timeout_ms);
uint8_t ata_read_status(uint8_t channel);
void ata_400ns_delay(uint8_t channel);
bool ata_polling(uint8_t channel, bool advanced_check);

// Error Handling
uint8_t ata_get_error(uint8_t channel);
const char* ata_get_error_string(uint8_t error);
void ata_print_error(AtaDevice* device);

// ATAPI Support (CD/DVD)
bool atapi_read_sectors(AtaDevice* device, uint32_t lba, uint16_t count, void* buffer);
bool atapi_eject(AtaDevice* device);

// IRQ Handlers
void ata_irq_handler_primary(void);
void ata_irq_handler_secondary(void);

// Device List Management (for your abstraction layer)
AtaDevice* ata_get_device(uint8_t index);
uint8_t ata_get_device_count(void);
AtaDevice* ata_get_device_by_path(uint8_t channel, uint8_t drive);

// Debug Functions
void ata_print_device_info(AtaDevice* device);
void ata_print_all_devices(void);

#ifdef __cplusplus
}
#endif