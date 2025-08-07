#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// ACPI Signature Definitions
#define ACPI_SIG_RSDP       "RSD PTR "
#define ACPI_SIG_RSDT       "RSDT"
#define ACPI_SIG_XSDT       "XSDT"
#define ACPI_SIG_FADT       "FACP"
#define ACPI_SIG_FACS       "FACS"
#define ACPI_SIG_DSDT       "DSDT"
#define ACPI_SIG_SSDT       "SSDT"
#define ACPI_SIG_MADT       "APIC"
#define ACPI_SIG_BERT       "BERT"
#define ACPI_SIG_CPEP       "CPEP"
#define ACPI_SIG_ECDT       "ECDT"
#define ACPI_SIG_EINJ       "EINJ"
#define ACPI_SIG_ERST       "ERST"
#define ACPI_SIG_HEST       "HEST"
#define ACPI_SIG_MSCT       "MSCT"
#define ACPI_SIG_MPST       "MPST"
#define ACPI_SIG_NFIT       "NFIT"
#define ACPI_SIG_PCCT       "PCCT"
#define ACPI_SIG_PHAT       "PHAT"
#define ACPI_SIG_PMTT       "PMTT"
#define ACPI_SIG_PSDT       "PSDT"
#define ACPI_SIG_RASF       "RASF"
#define ACPI_SIG_SBST       "SBST"
#define ACPI_SIG_SDEV       "SDEV"
#define ACPI_SIG_SLIT       "SLIT"
#define ACPI_SIG_SRAT       "SRAT"
#define ACPI_SIG_AEST       "AEST"
#define ACPI_SIG_BDAT       "BDAT"
#define ACPI_SIG_CDAT       "CDAT"
#define ACPI_SIG_CEDT       "CEDT"
#define ACPI_SIG_CRAT       "CRAT"
#define ACPI_SIG_CSRT       "CSRT"
#define ACPI_SIG_DBG2       "DBG2"
#define ACPI_SIG_DBGP       "DBGP"
#define ACPI_SIG_DMAR       "DMAR"
#define ACPI_SIG_DRTM       "DRTM"
#define ACPI_SIG_FPDT       "FPDT"
#define ACPI_SIG_GTDT       "GTDT"
#define ACPI_SIG_HMAT       "HMAT"
#define ACPI_SIG_HPET       "HPET"
#define ACPI_SIG_IBFT       "IBFT"
#define ACPI_SIG_IORT       "IORT"
#define ACPI_SIG_IVRS       "IVRS"
#define ACPI_SIG_LPIT       "LPIT"
#define ACPI_SIG_MCFG       "MCFG"
#define ACPI_SIG_MCHI       "MCHI"
#define ACPI_SIG_MPAM       "MPAM"
#define ACPI_SIG_MSDM       "MSDM"
#define ACPI_SIG_PRMT       "PRMT"
#define ACPI_SIG_RGRT       "RGRT"
#define ACPI_SIG_SDEI       "SDEI"
#define ACPI_SIG_SPCR       "SPCR"
#define ACPI_SIG_SPMI       "SPMI"
#define ACPI_SIG_STAO       "STAO"
#define ACPI_SIG_SVKL       "SVKL"
#define ACPI_SIG_TCPA       "TCPA"
#define ACPI_SIG_TPM2       "TPM2"
#define ACPI_SIG_UEFI       "UEFI"
#define ACPI_SIG_WAET       "WAET"
#define ACPI_SIG_WDAT       "WDAT"
#define ACPI_SIG_WDDT       "WDDT"
#define ACPI_SIG_WDRT       "WDRT"
#define ACPI_SIG_WPBT       "WPBT"
#define ACPI_SIG_WSMT       "WSMT"
#define ACPI_SIG_XENV       "XENV"

// ACPI Common Header Structure
typedef struct {
    char signature[4];          // Table signature
    uint32_t length;            // Length of table in bytes
    uint8_t revision;           // Revision of the structure
    uint8_t checksum;           // Checksum of entire table
    char oem_id[6];             // OEM identification
    char oem_table_id[8];       // OEM table identification
    uint32_t oem_revision;      // OEM revision number
    char asl_compiler_id[4];    // ASL compiler vendor ID
    uint32_t asl_compiler_revision; // ASL compiler revision number
} __attribute__((packed)) acpi_table_header_t;

// Root System Description Pointer (RSDP) - Version 1.0
typedef struct {
    char signature[8];          // "RSD PTR "
    uint8_t checksum;           // Checksum of RSDP v1.0 fields
    char oem_id[6];             // OEM identification
    uint8_t revision;           // ACPI revision (0 for v1.0, 2 for v2.0+)
    uint32_t rsdt_address;      // Physical address of RSDT
} __attribute__((packed)) acpi_rsdp_v1_t;

// Root System Description Pointer (RSDP) - Version 2.0+
typedef struct {
    acpi_rsdp_v1_t v1;          // Version 1.0 structure
    uint32_t length;            // Length of entire RSDP structure
    uint64_t xsdt_address;      // 64-bit physical address of XSDT
    uint8_t extended_checksum;  // Checksum of entire RSDP structure
    uint8_t reserved[3];        // Reserved for future use
} __attribute__((packed)) acpi_rsdp_v2_t;

// Root System Description Table (RSDT) - Version 1.0
typedef struct {
    acpi_table_header_t header;
    uint32_t table_offset_array[]; // Array of 32-bit physical addresses
} __attribute__((packed)) acpi_rsdt_t;

// Extended System Description Table (XSDT) - Version 2.0+
typedef struct {
    acpi_table_header_t header;
    uint64_t table_offset_array[]; // Array of 64-bit physical addresses
} __attribute__((packed)) acpi_xsdt_t;

// Fixed ACPI Description Table (FADT)
typedef struct {
    acpi_table_header_t header;
    uint32_t firmware_ctrl;     // Physical address of FACS
    uint32_t dsdt;              // Physical address of DSDT
    uint8_t reserved1;          // System interrupt model (not used in ACPI 2.0+)
    uint8_t preferred_profile;  // Conveys preferred power management profile
    uint16_t sci_interrupt;     // System Control Interrupt
    uint32_t smi_command;       // SMI command port
    uint8_t acpi_enable;        // Value to write to enable ACPI
    uint8_t acpi_disable;       // Value to write to disable ACPI
    uint8_t s4_bios_request;    // Value to write for S4 BIOS request
    uint8_t pstate_control;     // Processor performance state control
    uint32_t pm1a_event_block;  // PM1a Event Register Block
    uint32_t pm1b_event_block;  // PM1b Event Register Block
    uint32_t pm1a_control_block; // PM1a Control Register Block
    uint32_t pm1b_control_block; // PM1b Control Register Block
    uint32_t pm2_control_block; // PM2 Control Register Block
    uint32_t pm_timer_block;    // PM Timer Control Register Block
    uint32_t gpe0_block;        // GPE0 Register Block
    uint32_t gpe1_block;        // GPE1 Register Block
    uint8_t pm1_event_length;   // Byte Length of PM1 Event Register Block
    uint8_t pm1_control_length; // Byte Length of PM1 Control Register Block
    uint8_t pm2_control_length; // Byte Length of PM2 Control Register Block
    uint8_t pm_timer_length;    // Byte Length of PM Timer Register Block
    uint8_t gpe0_length;        // Byte Length of GPE0 Register Block
    uint8_t gpe1_length;        // Byte Length of GPE1 Register Block
    uint8_t gpe1_base;          // GPE1 Base Offset
    uint8_t cstate_control;     // C State Control
    uint16_t worst_c2_latency;  // Worst Case C2 Latency
    uint16_t worst_c3_latency;  // Worst Case C3 Latency
    uint16_t flush_size;        // Cache Flush Size
    uint16_t flush_stride;      // Cache Flush Stride
    uint8_t duty_offset;        // Duty Cycle Offset
    uint8_t duty_width;         // Duty Cycle Width
    uint8_t day_alarm;          // Day Alarm Index
    uint8_t month_alarm;        // Month Alarm Index
    uint8_t century;            // Century Index
    uint16_t boot_flags;        // IA-PC Boot Architecture Flags
    uint8_t reserved2;          // Reserved
    uint32_t flags;             // Fixed Feature Flags
    // ACPI 2.0+ fields
    uint8_t reset_reg[12];      // Reset Register (Generic Address Structure)
    uint8_t reset_value;        // Reset Value
    uint16_t arm_boot_flags;    // ARM Boot Architecture Flags
    uint8_t minor_revision;     // FADT Minor Revision
    uint64_t x_firmware_ctrl;   // 64-bit address of FACS
    uint64_t x_dsdt;            // 64-bit address of DSDT
    uint8_t x_pm1a_event_block[12]; // PM1a Event Register Block (GAS)
    uint8_t x_pm1b_event_block[12]; // PM1b Event Register Block (GAS)
    uint8_t x_pm1a_control_block[12]; // PM1a Control Register Block (GAS)
    uint8_t x_pm1b_control_block[12]; // PM1b Control Register Block (GAS)
    uint8_t x_pm2_control_block[12]; // PM2 Control Register Block (GAS)
    uint8_t x_pm_timer_block[12]; // PM Timer Control Register Block (GAS)
    uint8_t x_gpe0_block[12];   // GPE0 Register Block (GAS)
    uint8_t x_gpe1_block[12];   // GPE1 Register Block (GAS)
    uint8_t sleep_control_reg[12]; // Sleep Control Register (GAS)
    uint8_t sleep_status_reg[12]; // Sleep Status Register (GAS)
    uint64_t hypervisor_vendor_id; // Hypervisor Vendor ID
} __attribute__((packed)) acpi_fadt_t;

// Firmware ACPI Control Structure (FACS)
typedef struct {
    char signature[4];          // "FACS"
    uint32_t length;            // Length of structure
    uint32_t hardware_signature; // Hardware configuration signature
    uint32_t firmware_waking_vector; // 32-bit firmware waking vector
    uint32_t global_lock;       // Global Lock
    uint32_t flags;             // Flags
    uint64_t x_firmware_waking_vector; // 64-bit firmware waking vector
    uint8_t version;            // Version of this structure
    uint8_t reserved[3];        // Reserved
    uint32_t ospm_flags;        // OSPM Enabled Firmware Control Structure Flags
    uint8_t reserved2[24];      // Reserved
} __attribute__((packed)) acpi_facs_t;

// Multiple APIC Description Table (MADT)
typedef struct {
    acpi_table_header_t header;
    uint32_t local_apic_address; // Physical address of local APIC
    uint32_t flags;             // Multiple APIC flags
    // Variable length array of APIC structures follows
} __attribute__((packed)) acpi_madt_t;

// MADT APIC Structure Types
#define ACPI_MADT_TYPE_LOCAL_APIC           0
#define ACPI_MADT_TYPE_IO_APIC              1
#define ACPI_MADT_TYPE_INTERRUPT_OVERRIDE   2
#define ACPI_MADT_TYPE_NMI_SOURCE           3
#define ACPI_MADT_TYPE_LOCAL_APIC_NMI       4
#define ACPI_MADT_TYPE_LOCAL_APIC_OVERRIDE  5
#define ACPI_MADT_TYPE_IO_SAPIC             6
#define ACPI_MADT_TYPE_LOCAL_SAPIC          7
#define ACPI_MADT_TYPE_INTERRUPT_SOURCE     8
#define ACPI_MADT_TYPE_LOCAL_X2APIC         9
#define ACPI_MADT_TYPE_LOCAL_X2APIC_NMI     10
#define ACPI_MADT_TYPE_GENERIC_INTERRUPT    11
#define ACPI_MADT_TYPE_GENERIC_DISTRIBUTOR  12
#define ACPI_MADT_TYPE_GENERIC_MSI_FRAME    13
#define ACPI_MADT_TYPE_GENERIC_REDISTRIBUTOR 14
#define ACPI_MADT_TYPE_GENERIC_TRANSLATOR   15

// MADT APIC Structure Header
typedef struct {
    uint8_t type;               // APIC structure type
    uint8_t length;             // Length of structure
} __attribute__((packed)) acpi_madt_apic_header_t;

// Local APIC Structure
typedef struct {
    acpi_madt_apic_header_t header;
    uint8_t processor_id;       // ACPI processor ID
    uint8_t apic_id;            // Processor's local APIC ID
    uint32_t flags;             // Local APIC flags
} __attribute__((packed)) acpi_madt_local_apic_t;

// I/O APIC Structure
typedef struct {
    acpi_madt_apic_header_t header;
    uint8_t io_apic_id;         // I/O APIC ID
    uint8_t reserved;           // Reserved
    uint32_t io_apic_address;   // I/O APIC address
    uint32_t global_system_interrupt_base; // Global system interrupt base
} __attribute__((packed)) acpi_madt_io_apic_t;

// Interrupt Source Override Structure
typedef struct {
    acpi_madt_apic_header_t header;
    uint8_t bus;                // Bus that the interrupt source belongs to
    uint8_t source;             // Bus-relative interrupt source
    uint32_t global_system_interrupt; // Global system interrupt
    uint16_t flags;             // MPS INTI flags
} __attribute__((packed)) acpi_madt_interrupt_override_t;

// High Precision Event Timer (HPET) Table
typedef struct {
    acpi_table_header_t header;
    uint32_t event_timer_block_id; // Event Timer Block ID
    uint8_t base_address[12];   // Base Address (Generic Address Structure)
    uint8_t hpet_number;        // HPET sequence number
    uint16_t minimum_tick;      // Main counter minimum clock tick
    uint8_t page_protection;    // Page protection and OEM attribute
} __attribute__((packed)) acpi_hpet_t;

// Memory Mapped Configuration Space (MCFG) Table
typedef struct {
    acpi_table_header_t header;
    uint64_t reserved;          // Reserved
    // Variable length array of configuration space allocations follows
} __attribute__((packed)) acpi_mcfg_t;

// MCFG Configuration Space Base Address Allocation Structure
typedef struct {
    uint64_t base_address;      // Base address of enhanced configuration space
    uint16_t pci_segment;       // PCI segment group number
    uint8_t start_bus_number;   // Start PCI bus number
    uint8_t end_bus_number;     // End PCI bus number
    uint32_t reserved;          // Reserved
} __attribute__((packed)) acpi_mcfg_allocation_t;

// System Locality Distance Information Table (SLIT)
typedef struct {
    acpi_table_header_t header;
    uint64_t locality_count;    // Number of system localities
    uint8_t entry[];            // Matrix of relative distances
} __attribute__((packed)) acpi_slit_t;

// System Resource Affinity Table (SRAT)
typedef struct {
    acpi_table_header_t header;
    uint32_t table_revision;    // Must be 1
    uint64_t reserved;          // Reserved
    // Variable length array of affinity structures follows
} __attribute__((packed)) acpi_srat_t;

// SRAT Structure Types
#define ACPI_SRAT_TYPE_CPU_AFFINITY         0
#define ACPI_SRAT_TYPE_MEMORY_AFFINITY      1
#define ACPI_SRAT_TYPE_X2APIC_CPU_AFFINITY  2
#define ACPI_SRAT_TYPE_GICC_AFFINITY        3
#define ACPI_SRAT_TYPE_GIC_ITS_AFFINITY     4
#define ACPI_SRAT_TYPE_GENERIC_AFFINITY     5

// SRAT CPU Affinity Structure
typedef struct {
    uint8_t type;               // Structure type (0)
    uint8_t length;             // Length of structure
    uint8_t proximity_domain_lo; // Proximity domain (low)
    uint8_t apic_id;            // Local APIC ID
    uint32_t flags;             // Flags
    uint8_t local_sapic_eid;    // Local SAPIC EID
    uint8_t proximity_domain_hi[3]; // Proximity domain (high)
    uint32_t clock_domain;      // Clock domain
} __attribute__((packed)) acpi_srat_cpu_affinity_t;

// SRAT Memory Affinity Structure
typedef struct {
    uint8_t type;               // Structure type (1)
    uint8_t length;             // Length of structure
    uint32_t proximity_domain;  // Proximity domain
    uint16_t reserved1;         // Reserved
    uint32_t base_address_low;  // Base address (low 32 bits)
    uint32_t base_address_high; // Base address (high 32 bits)
    uint32_t length_low;        // Range length (low 32 bits)
    uint32_t length_high;       // Range length (high 32 bits)
    uint32_t reserved2;         // Reserved
    uint32_t flags;             // Flags
    uint64_t reserved3;         // Reserved
} __attribute__((packed)) acpi_srat_memory_affinity_t;

// Boot Error Record Table (BERT)
typedef struct {
    acpi_table_header_t header;
    uint32_t boot_error_region_length; // Boot Error Region Length
    uint64_t boot_error_region;        // Boot Error Region Address
} __attribute__((packed)) acpi_bert_t;

// Corrected Platform Error Polling Table (CPEP)
typedef struct {
    acpi_table_header_t header;
    uint64_t reserved;          // Reserved
    // Variable length array of CPEP processor info structures follows
} __attribute__((packed)) acpi_cpep_t;

// Embedded Controller Boot Resources Table (ECDT)
typedef struct {
    acpi_table_header_t header;
    uint8_t ec_control[12];     // EC Control Register (Generic Address Structure)
    uint8_t ec_data[12];        // EC Data Register (Generic Address Structure)
    uint32_t uid;               // Unique ID
    uint8_t gpe_bit;            // GPE bit
    char ec_id[];               // Null-terminated ASCII string
} __attribute__((packed)) acpi_ecdt_t;

// Error Injection Table (EINJ)
typedef struct {
    acpi_table_header_t header;
    uint32_t header_length;     // Injection Header Length
    uint8_t flags;              // Flags
    uint8_t reserved[3];        // Reserved
    uint32_t entries;           // Number of injection entries
    // Variable length array of injection instruction entries follows
} __attribute__((packed)) acpi_einj_t;

// Error Record Serialization Table (ERST)
typedef struct {
    acpi_table_header_t header;
    uint32_t header_length;     // Serialization Header Length
    uint32_t reserved;          // Reserved
    uint32_t entries;           // Number of serialization entries
    // Variable length array of serialization instruction entries follows
} __attribute__((packed)) acpi_erst_t;

// Hardware Error Source Table (HEST)
typedef struct {
    acpi_table_header_t header;
    uint32_t error_source_count; // Number of error sources
    // Variable length array of error source descriptors follows
} __attribute__((packed)) acpi_hest_t;

// Maximum System Characteristics Table (MSCT)
typedef struct {
    acpi_table_header_t header;
    uint32_t proximity_offset;  // Offset to proximity domain array
    uint32_t max_proximity_domains; // Maximum number of proximity domains
    uint32_t max_clock_domains; // Maximum number of clock domains
    uint64_t max_address;       // Maximum physical address
    // Variable length arrays follow
} __attribute__((packed)) acpi_msct_t;

// Serial Port Console Redirection Table (SPCR)
typedef struct {
    acpi_table_header_t header;
    uint8_t interface_type;     // Interface type
    uint8_t reserved[3];        // Reserved
    uint8_t serial_port[12];    // Serial Port Register (Generic Address Structure)
    uint8_t interrupt_type;     // Interrupt type
    uint8_t pc_interrupt;       // PC-AT compatible IRQ
    uint32_t interrupt;         // Interrupt
    uint8_t baud_rate;          // Baud rate
    uint8_t parity;             // Parity
    uint8_t stop_bits;          // Stop bits
    uint8_t flow_control;       // Flow control
    uint8_t terminal_type;      // Terminal type
    uint8_t reserved2;          // Reserved
    uint16_t pci_device_id;     // PCI device ID
    uint16_t pci_vendor_id;     // PCI vendor ID
    uint8_t pci_bus;            // PCI bus number
    uint8_t pci_device;         // PCI device number
    uint8_t pci_function;       // PCI function number
    uint32_t pci_flags;         // PCI flags
    uint8_t pci_segment;        // PCI segment
    uint32_t reserved3;         // Reserved
} __attribute__((packed)) acpi_spcr_t;

// Service Processor Management Interface Table (SPMI)
typedef struct {
    acpi_table_header_t header;
    uint8_t interface_type;     // Interface type
    uint8_t reserved;           // Reserved
    uint16_t spec_revision;     // IPMI specification revision
    uint8_t interrupt_type;     // Interrupt type
    uint8_t gpe;                // GPE number
    uint8_t reserved2;          // Reserved
    uint8_t pci_device_flag;    // PCI device flag
    uint32_t interrupt;         // Interrupt
    uint8_t ipmi_register[12];  // IPMI Register (Generic Address Structure)
    uint8_t pci_segment;        // PCI segment
    uint8_t pci_bus;            // PCI bus
    uint8_t pci_device;         // PCI device
    uint8_t pci_function;       // PCI function
    uint8_t reserved3;          // Reserved
} __attribute__((packed)) acpi_spmi_t;

// Trusted Computing Platform Alliance Capabilities Table (TCPA)
typedef struct {
    acpi_table_header_t header;
    uint16_t platform_class;    // Platform class
    uint32_t log_max_length;    // Maximum length of event log
    uint64_t log_address;       // Address of event log
} __attribute__((packed)) acpi_tcpa_t;

// Trusted Platform Module 2 Table (TPM2)
typedef struct {
    acpi_table_header_t header;
    uint16_t platform_class;    // Platform class
    uint16_t reserved;          // Reserved
    uint64_t control_address;   // Address of control area
    uint32_t start_method;      // Start method
    // Method-specific parameters follow
} __attribute__((packed)) acpi_tpm2_t;

// Windows ACPI Enlightenment Table (WAET)
typedef struct {
    acpi_table_header_t header;
    uint32_t flags;             // Flags
} __attribute__((packed)) acpi_waet_t;

// Watchdog Action Table (WDAT)
typedef struct {
    acpi_table_header_t header;
    uint32_t header_length;     // Watchdog Header Length
    uint16_t pci_segment;       // PCI segment
    uint8_t pci_bus;            // PCI bus
    uint8_t pci_device;         // PCI device
    uint8_t pci_function;       // PCI function
    uint8_t reserved[3];        // Reserved
    uint32_t timer_period;      // Timer period
    uint32_t max_count;         // Maximum count
    uint32_t min_count;         // Minimum count
    uint8_t flags;              // Flags
    uint8_t reserved2[3];       // Reserved
    uint32_t entries;           // Number of watchdog instruction entries
    // Variable length array of watchdog instruction entries follows
} __attribute__((packed)) acpi_wdat_t;

// Watchdog Descriptor Table (WDDT)
typedef struct {
    acpi_table_header_t header;
    uint16_t spec_version;      // Specification version
    uint16_t table_version;     // Table version
    uint16_t pci_vendor_id;     // PCI vendor ID
    uint8_t address[12];        // Timer Register (Generic Address Structure)
    uint16_t max_count;         // Maximum count
    uint16_t min_count;         // Minimum count
    uint16_t period;            // Timer period
    uint16_t status;            // Status
    uint16_t capability;        // Capability
} __attribute__((packed)) acpi_wddt_t;

// Watchdog Resource Table (WDRT)
typedef struct {
    acpi_table_header_t header;
    uint8_t control_register[12]; // Control Register (Generic Address Structure)
    uint8_t count_register[12];   // Count Register (Generic Address Structure)
    uint16_t pci_device_id;     // PCI device ID
    uint16_t pci_vendor_id;     // PCI vendor ID
    uint8_t pci_bus;            // PCI bus number
    uint8_t pci_device;         // PCI device number
    uint8_t pci_function;       // PCI function number
    uint8_t pci_segment;        // PCI segment
    uint16_t max_count;         // Maximum count
    uint8_t units;              // Count units
} __attribute__((packed)) acpi_wdrt_t;

// Windows Platform Binary Table (WPBT)
typedef struct {
    acpi_table_header_t header;
    uint32_t handoff_size;      // Handoff memory size
    uint64_t handoff_address;   // Handoff memory address
    uint8_t layout;             // Layout
    uint8_t type;               // Type
    uint16_t arguments_length;  // Arguments length
    // Variable length data follows
} __attribute__((packed)) acpi_wpbt_t;

// Windows SMM Security Mitigations Table (WSMT)
typedef struct {
    acpi_table_header_t header;
    uint32_t protection_flags;  // Protection flags
} __attribute__((packed)) acpi_wsmt_t;

// Generic Address Structure (GAS)
typedef struct {
    uint8_t space_id;           // Address space ID
    uint8_t bit_width;          // Register bit width
    uint8_t bit_offset;         // Register bit offset
    uint8_t access_size;        // Access size
    uint64_t address;           // Register address
} __attribute__((packed)) acpi_gas_t;

// ACPI Address Space IDs
#define ACPI_ADR_SPACE_SYSTEM_MEMORY    0
#define ACPI_ADR_SPACE_SYSTEM_IO        1
#define ACPI_ADR_SPACE_PCI_CONFIG       2
#define ACPI_ADR_SPACE_EC               3
#define ACPI_ADR_SPACE_SMBUS            4
#define ACPI_ADR_SPACE_CMOS             5
#define ACPI_ADR_SPACE_PCI_BAR_TARGET   6
#define ACPI_ADR_SPACE_IPMI             7
#define ACPI_ADR_SPACE_GPIO             8
#define ACPI_ADR_SPACE_GSBUS            9
#define ACPI_ADR_SPACE_PLATFORM_COMM    10

// ACPI Function Prototypes
bool acpi_validate_checksum(void *table, size_t length);
acpi_rsdp_v1_t* acpi_find_rsdp(void);
acpi_table_header_t* acpi_find_table(const char *signature);
void acpi_parse_madt(acpi_madt_t *madt);
void acpi_parse_fadt(acpi_fadt_t *fadt);
void acpi_init(void);

#ifdef __cplusplus
}
#endif
