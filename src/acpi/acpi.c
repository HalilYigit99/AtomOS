#include <acpi/acpi.h>
#include <boot/multiboot2.h>
#include <string.h>

// =============================================================================
// ACPI Global Pointers
// =============================================================================
acpi_rsdp_v2_t *acpi_rsdp = NULL;
acpi_rsdt_t *acpi_rsdt = NULL;
acpi_xsdt_t *acpi_xsdt = NULL;
acpi_fadt_t *acpi_fadt = NULL;
acpi_facs_t *acpi_facs = NULL;
acpi_dsdt_t *acpi_dsdt = NULL;
acpi_madt_t *acpi_madt = NULL;
acpi_hpet_t *acpi_hpet = NULL;
acpi_mcfg_t *acpi_mcfg = NULL;
acpi_srat_t *acpi_srat = NULL;
acpi_slit_t *acpi_slit = NULL;
acpi_bert_t *acpi_bert = NULL;

/**
 * Initialize the ACPI subsystem
 * 
 * This function will be responsible for:
 * - Finding and validating the RSDP (Root System Description Pointer)
 * - Locating the RSDT/XSDT
 * - Discovering and caching important ACPI tables
 * - Setting up ACPI-related hardware
 * 
 * Currently this is a stub implementation.
 */
void acpi_init(void) {
    // TODO: Implement ACPI initialization
    // 
    // Implementation steps:
    // 1. Find RSDP through EFI or BIOS scan |OK|
    // 2. Validate RSDP checksum |OK|
    // 3. Get RSDT/XSDT address from RSDP |OK|
    // 4. Validate and parse RSDT/XSDT |OK|
    // 5. Discover and cache important tables (FADT, MADT, HPET, MCFG, etc.)
    // 6. Parse critical tables for system configuration
    // 7. Enable ACPI mode if needed

    if (mb2_acpi_old) {
        acpi_rsdp = (acpi_rsdp_v2_t *)mb2_acpi_old->rsdp;
    }

    if (mb2_acpi_new) {
        acpi_rsdp = (acpi_rsdp_v2_t *)mb2_acpi_new->rsdp;
    }

    // Validate checksum of RSDP
    // Validate RSDP checksum
    if (acpi_rsdp) {
        uint8_t checksum = 0;
        uint8_t *ptr = (uint8_t *)acpi_rsdp;
        
        // Check RSDP v1.0 checksum (first 20 bytes)
        for (int i = 0; i < 20; i++) {
            checksum += ptr[i];
        }
        
        if (checksum != 0) {
            acpi_rsdp = NULL;
            return;
        }
        
        // If RSDP v2.0, check extended checksum
        if (acpi_rsdp->v1.revision >= 2) {
            checksum = 0;
            for (int i = 0; i < acpi_rsdp->length; i++) {
                checksum += ptr[i];
            }
            
            if (checksum != 0) {
                acpi_rsdp = NULL;
                return;
            }
            
            // Get XSDT from RSDP v2.0
            acpi_xsdt = (acpi_xsdt_t *)acpi_rsdp->xsdt_address;
        } else {
            // Get RSDT from RSDP v1.0
            acpi_rsdt = (acpi_rsdt_t *)acpi_rsdp->v1.rsdt_address;
        }
    }

    // Validate XSDT if available
    if (acpi_xsdt) {
        uint8_t checksum = 0;
        uint8_t *ptr = (uint8_t *)acpi_xsdt;
        
        for (uint32_t i = 0; i < acpi_xsdt->header.length; i++) {
            checksum += ptr[i];
        }
        
        if (checksum != 0) {
            acpi_xsdt = NULL;
        }
    }

    // Validate RSDT if available
    if (acpi_rsdt) {
        uint8_t checksum = 0;
        uint8_t *ptr = (uint8_t *)acpi_rsdt;
        
        for (uint32_t i = 0; i < acpi_rsdt->header.length; i++) {
            checksum += ptr[i];
        }
        
        if (checksum != 0) {
            acpi_rsdt = NULL;
        }
    }

    // 5. Discover and cache important tables (FADT, MADT, HPET, MCFG, etc.)
    
    

}
