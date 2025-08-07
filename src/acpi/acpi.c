#include <acpi/acpi.h>
#include <boot/multiboot2.h>
#include <efi/efi.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stream/OutputStream.h>

// Global ACPI table pointers
static acpi_rsdp_v1_t* g_rsdp = NULL;
static acpi_rsdp_v2_t* g_rsdp_v2 = NULL;
static acpi_rsdt_t* g_rsdt = NULL;
static acpi_xsdt_t* g_xsdt = NULL;
static acpi_fadt_t* g_fadt = NULL;
static acpi_madt_t* g_madt = NULL;
static acpi_hpet_t* g_hpet = NULL;
static acpi_mcfg_t* g_mcfg = NULL;
static acpi_srat_t* g_srat = NULL;
static acpi_slit_t* g_slit = NULL;

// ACPI version (1 for ACPI 1.0, 2+ for ACPI 2.0+)
static uint8_t g_acpi_version = 0;

// EFI GUID for ACPI tables
static const EFI_GUID EFI_ACPI_TABLE_GUID = 
    {0x8868E871, 0xE4F1, 0x11D3, {0xBC, 0x22, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}};
static const EFI_GUID EFI_ACPI_20_TABLE_GUID = 
    {0xEB9D2D30, 0x2D88, 0x11D3, {0x9A, 0x16, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}};

// Validate ACPI table checksum
bool acpi_validate_checksum(void *table, size_t length) {
    uint8_t sum = 0;
    uint8_t *bytes = (uint8_t*)table;
    
    for (size_t i = 0; i < length; i++) {
        sum += bytes[i];
    }
    
    return sum == 0;
}

// Compare two GUIDs
static bool guid_compare(const EFI_GUID *guid1, const EFI_GUID *guid2) {
    return guid1->data1 == guid2->data1 &&
           guid1->data2 == guid2->data2 &&
           guid1->data3 == guid2->data3 &&
           memcmp(guid1->data4, guid2->data4, 8) == 0;
}

// Find RSDP through EFI System Table
static acpi_rsdp_v1_t* acpi_find_rsdp_efi(void) {
    if (!efi_system_table || !efi_system_table->configuration_table) {
        currentOutputStream->printf("EFI System Table not available.\n");
        return NULL;
    }
    
    // Search through EFI configuration tables
    for (uint64_t i = 0; i < efi_system_table->number_of_table_entries; i++) {
        EFI_CONFIGURATION_TABLE *table = &efi_system_table->configuration_table[i];
        
        // Check for ACPI 2.0+ table first (preferred)
        if (guid_compare(&table->vendor_guid, &EFI_ACPI_20_TABLE_GUID)) {
            acpi_rsdp_v2_t *rsdp_v2 = (acpi_rsdp_v2_t*)table->vendor_table;
            if (rsdp_v2 && memcmp(rsdp_v2->v1.signature, ACPI_SIG_RSDP, 8) == 0) {
                // Validate checksum for v2
                if (acpi_validate_checksum(&rsdp_v2->v1, sizeof(acpi_rsdp_v1_t))) {
                    if (rsdp_v2->v1.revision >= 2) {
                        // Also validate extended checksum for v2
                        if (acpi_validate_checksum(rsdp_v2, rsdp_v2->length)) {
                            g_rsdp_v2 = rsdp_v2;
                            g_acpi_version = rsdp_v2->v1.revision;
                            return &rsdp_v2->v1;
                        }
                    }
                }
            }
        }
        
        // Check for ACPI 1.0 table
        if (guid_compare(&table->vendor_guid, &EFI_ACPI_TABLE_GUID)) {
            acpi_rsdp_v1_t *rsdp = (acpi_rsdp_v1_t*)table->vendor_table;
            if (rsdp && memcmp(rsdp->signature, ACPI_SIG_RSDP, 8) == 0) {
                // Validate checksum
                if (acpi_validate_checksum(rsdp, sizeof(acpi_rsdp_v1_t))) {
                    g_acpi_version = rsdp->revision;
                    return rsdp;
                }
            }
        }
    }
    
    return NULL;
}

// Find RSDP through BIOS memory scan
static acpi_rsdp_v1_t* acpi_find_rsdp_bios(void) {
    // RSDP is located in one of two areas:
    // 1. First 1KB of EBDA (Extended BIOS Data Area)
    // 2. Between 0xE0000 and 0xFFFFF (BIOS ROM area)
    
    // Get EBDA base address from BDA (BIOS Data Area)
    uint16_t ebda_segment = *((uint16_t*)0x40E);
    uintptr_t ebda_base = (uintptr_t)ebda_segment << 4;
    
    // Search in EBDA first (if valid)
    if (ebda_base >= 0x80000 && ebda_base < 0xA0000) {
        for (uintptr_t addr = ebda_base; addr < ebda_base + 1024; addr += 16) {
            acpi_rsdp_v1_t *rsdp = (acpi_rsdp_v1_t*)addr;
            if (memcmp(rsdp->signature, ACPI_SIG_RSDP, 8) == 0) {
                // Validate checksum
                if (acpi_validate_checksum(rsdp, sizeof(acpi_rsdp_v1_t))) {
                    g_acpi_version = rsdp->revision;
                    
                    // Check if this is actually v2
                    if (rsdp->revision >= 2) {
                        acpi_rsdp_v2_t *rsdp_v2 = (acpi_rsdp_v2_t*)rsdp;
                        if (acpi_validate_checksum(rsdp_v2, rsdp_v2->length)) {
                            g_rsdp_v2 = rsdp_v2;
                        }
                    }
                    
                    return rsdp;
                }
            }
        }
    }
    
    // Search in BIOS ROM area
    for (uintptr_t addr = 0xE0000; addr < 0x100000; addr += 16) {
        acpi_rsdp_v1_t *rsdp = (acpi_rsdp_v1_t*)addr;
        if (memcmp(rsdp->signature, ACPI_SIG_RSDP, 8) == 0) {
            // Validate checksum
            if (acpi_validate_checksum(rsdp, sizeof(acpi_rsdp_v1_t))) {
                g_acpi_version = rsdp->revision;
                
                // Check if this is actually v2
                if (rsdp->revision >= 2) {
                    acpi_rsdp_v2_t *rsdp_v2 = (acpi_rsdp_v2_t*)rsdp;
                    if (acpi_validate_checksum(rsdp_v2, rsdp_v2->length)) {
                        g_rsdp_v2 = rsdp_v2;
                    }
                }
                
                return rsdp;
            }
        }
    }
    
    return NULL;
}

// Public function to find RSDP
acpi_rsdp_v1_t* acpi_find_rsdp(void) {
    if (g_rsdp) {
        return g_rsdp;
    }
    
    if (mb2_is_efi_boot) {
        g_rsdp = acpi_find_rsdp_efi();
    } else {
        g_rsdp = acpi_find_rsdp_bios();
    }
    
    return g_rsdp;
}

// Find a specific ACPI table by signature
acpi_table_header_t* acpi_find_table(const char *signature) {
    if (!g_rsdp) {
        if (!acpi_find_rsdp()) {
            return NULL;
        }
    }
    
    // Use XSDT if available (ACPI 2.0+), otherwise use RSDT
    if (g_xsdt) {
        uint32_t entries = (g_xsdt->header.length - sizeof(acpi_table_header_t)) / sizeof(uint64_t);
        
        for (uint32_t i = 0; i < entries; i++) {
            acpi_table_header_t *table = (acpi_table_header_t*)(uintptr_t)g_xsdt->table_offset_array[i];
            
            if (table && memcmp(table->signature, signature, 4) == 0) {
                // Validate checksum
                if (acpi_validate_checksum(table, table->length)) {
                    return table;
                }
            }
        }
    } else if (g_rsdt) {
        uint32_t entries = (g_rsdt->header.length - sizeof(acpi_table_header_t)) / sizeof(uint32_t);
        
        for (uint32_t i = 0; i < entries; i++) {
            acpi_table_header_t *table = (acpi_table_header_t*)(uintptr_t)g_rsdt->table_offset_array[i];
            
            if (table && memcmp(table->signature, signature, 4) == 0) {
                // Validate checksum
                if (acpi_validate_checksum(table, table->length)) {
                    return table;
                }
            }
        }
    }
    
    return NULL;
}

// Parse MADT (Multiple APIC Description Table)
void acpi_parse_madt(acpi_madt_t *madt) {
    if (!madt) return;
    
    // Parse MADT entries
    uint8_t *ptr = (uint8_t*)madt + sizeof(acpi_madt_t);
    uint8_t *end = (uint8_t*)madt + madt->header.length;
    
    while (ptr < end) {
        acpi_madt_apic_header_t *header = (acpi_madt_apic_header_t*)ptr;
        
        switch (header->type) {
            case ACPI_MADT_TYPE_LOCAL_APIC: {
                acpi_madt_local_apic_t *lapic = (acpi_madt_local_apic_t*)ptr;
                // Process Local APIC
                // TODO: Store LAPIC information
                break;
            }
            
            case ACPI_MADT_TYPE_IO_APIC: {
                acpi_madt_io_apic_t *ioapic = (acpi_madt_io_apic_t*)ptr;
                // Process I/O APIC
                // TODO: Store I/O APIC information
                break;
            }
            
            case ACPI_MADT_TYPE_INTERRUPT_OVERRIDE: {
                acpi_madt_interrupt_override_t *override = (acpi_madt_interrupt_override_t*)ptr;
                // Process Interrupt Override
                // TODO: Store interrupt override information
                break;
            }
            
            // Add more cases as needed
        }
        
        ptr += header->length;
    }
}

// Parse FADT (Fixed ACPI Description Table)
void acpi_parse_fadt(acpi_fadt_t *fadt) {
    if (!fadt) return;
    
    // Store important FADT information
    // TODO: Process FADT fields as needed
    
    // Get DSDT address
    uint64_t dsdt_addr = 0;
    if (fadt->x_dsdt && g_acpi_version >= 2) {
        dsdt_addr = fadt->x_dsdt;
    } else if (fadt->dsdt) {
        dsdt_addr = fadt->dsdt;
    }
    
    if (dsdt_addr) {
        acpi_table_header_t *dsdt = (acpi_table_header_t*)(uintptr_t)dsdt_addr;
        // TODO: Process DSDT if needed
    }
    
    // Get FACS address
    uint64_t facs_addr = 0;
    if (fadt->x_firmware_ctrl && g_acpi_version >= 2) {
        facs_addr = fadt->x_firmware_ctrl;
    } else if (fadt->firmware_ctrl) {
        facs_addr = fadt->firmware_ctrl;
    }
    
    if (facs_addr) {
        acpi_facs_t *facs = (acpi_facs_t*)(uintptr_t)facs_addr;
        // TODO: Process FACS if needed
    }
}

// Initialize ACPI subsystem
void acpi_init(void) {
    // Find RSDP first
    if (mb2_is_efi_boot) {
        // EFI boot path
        g_rsdp = acpi_find_rsdp_efi();
        if (!g_rsdp) {
            // Failed to find RSDP through EFI
            currentOutputStream->printf("Failed to find RSDP through EFI.\n");
            return;
        }
    } else {
        // BIOS boot path
        g_rsdp = acpi_find_rsdp_bios();
        if (!g_rsdp) {
            // Failed to find RSDP through BIOS scan
            currentOutputStream->printf("Failed to find RSDP through BIOS scan.\n");
            return;
        }
    }

    currentOutputStream->printf("ACPI Version: %u\n", (size_t)g_acpi_version);
    
    // Get RSDT/XSDT address
    if (g_acpi_version >= 2 && g_rsdp_v2) {
        // Use XSDT for ACPI 2.0+
        if (g_rsdp_v2->xsdt_address) {
            g_xsdt = (acpi_xsdt_t*)(uintptr_t)g_rsdp_v2->xsdt_address;
            
            // Validate XSDT
            if (!acpi_validate_checksum(g_xsdt, g_xsdt->header.length)) {
                g_xsdt = NULL;
            }
        }
    }
    
    // Fall back to RSDT if XSDT is not available
    if (!g_xsdt && g_rsdp->rsdt_address) {
        g_rsdt = (acpi_rsdt_t*)(uintptr_t)g_rsdp->rsdt_address;
        
        // Validate RSDT
        if (!acpi_validate_checksum(g_rsdt, g_rsdt->header.length)) {
            g_rsdt = NULL;
            return;
        }
    }
    
    // Find and cache important tables
    g_fadt = (acpi_fadt_t*)acpi_find_table(ACPI_SIG_FADT);
    g_madt = (acpi_madt_t*)acpi_find_table(ACPI_SIG_MADT);
    g_hpet = (acpi_hpet_t*)acpi_find_table(ACPI_SIG_HPET);
    g_mcfg = (acpi_mcfg_t*)acpi_find_table(ACPI_SIG_MCFG);
    g_srat = (acpi_srat_t*)acpi_find_table(ACPI_SIG_SRAT);
    g_slit = (acpi_slit_t*)acpi_find_table(ACPI_SIG_SLIT);
    
    // Parse important tables
    if (g_fadt) {
        acpi_parse_fadt(g_fadt);
    }
    
    if (g_madt) {
        acpi_parse_madt(g_madt);
    }
    
    // Additional table parsing can be added here
    
}

// Getter functions for global table pointers
acpi_fadt_t* acpi_get_fadt(void) { return g_fadt; }
acpi_madt_t* acpi_get_madt(void) { return g_madt; }
acpi_hpet_t* acpi_get_hpet(void) { return g_hpet; }
acpi_mcfg_t* acpi_get_mcfg(void) { return g_mcfg; }
acpi_srat_t* acpi_get_srat(void) { return g_srat; }
acpi_slit_t* acpi_get_slit(void) { return g_slit; }
uint8_t acpi_get_version(void) { return g_acpi_version; }