#include <efi/efi.h>

EFI_SYSTEM_TABLE* efi_system_table = NULL;
EFI_HANDLE efi_image_handle = NULL;

void efi_init(void) {

    if (!efi_image_handle || !efi_system_table) {
        // If EFI image handle or system table is not set, we cannot proceed
        return;
    }

}

