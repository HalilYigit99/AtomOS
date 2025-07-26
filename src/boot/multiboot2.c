#include <boot/multiboot2.h>
#include <stddef.h>

/* Tag pointer definitions */
struct multiboot_tag_string *mb2_cmdline = NULL;
struct multiboot_tag_string *mb2_bootloader_name = NULL;
struct multiboot_tag_module *mb2_module = NULL;
struct multiboot_tag_basic_meminfo *mb2_basic_meminfo = NULL;
struct multiboot_tag_bootdev *mb2_bootdev = NULL;
struct multiboot_tag_mmap *mb2_mmap = NULL;
struct multiboot_tag_framebuffer *mb2_framebuffer = NULL;
struct multiboot_tag_elf_sections *mb2_elf_sections = NULL;
struct multiboot_tag_apm *mb2_apm = NULL;
struct multiboot_tag_efi32 *mb2_efi32 = NULL;
struct multiboot_tag_efi64 *mb2_efi64 = NULL;
struct multiboot_tag_smbios *mb2_smbios = NULL;
struct multiboot_tag_old_acpi *mb2_acpi_old = NULL;
struct multiboot_tag_new_acpi *mb2_acpi_new = NULL;
struct multiboot_tag_network *mb2_network = NULL;
struct multiboot_tag_efi_mmap *mb2_efi_mmap = NULL;
struct multiboot_tag_efi32_ih *mb2_efi32_ih = NULL;
struct multiboot_tag_efi64_ih *mb2_efi64_ih = NULL;
struct multiboot_tag_load_base_addr *mb2_load_base_addr = NULL;

void multiboot2_parse(uintptr_t mb2_info_addr) {
    struct multiboot_tag *tag;
    
    /* Skip the first 8 bytes (total_size and reserved) */
    tag = (struct multiboot_tag *)(mb2_info_addr + 8);
    
    /* Parse all tags */
    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                if (!mb2_cmdline) {
                    mb2_cmdline = (struct multiboot_tag_string *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                if (!mb2_bootloader_name) {
                    mb2_bootloader_name = (struct multiboot_tag_string *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_MODULE:
                if (!mb2_module) {
                    mb2_module = (struct multiboot_tag_module *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                if (!mb2_basic_meminfo) {
                    mb2_basic_meminfo = (struct multiboot_tag_basic_meminfo *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_BOOTDEV:
                if (!mb2_bootdev) {
                    mb2_bootdev = (struct multiboot_tag_bootdev *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_MMAP:
                if (!mb2_mmap) {
                    mb2_mmap = (struct multiboot_tag_mmap *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
                if (!mb2_framebuffer) {
                    mb2_framebuffer = (struct multiboot_tag_framebuffer *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
                if (!mb2_elf_sections) {
                    mb2_elf_sections = (struct multiboot_tag_elf_sections *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_APM:
                if (!mb2_apm) {
                    mb2_apm = (struct multiboot_tag_apm *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_EFI32:
                if (!mb2_efi32) {
                    mb2_efi32 = (struct multiboot_tag_efi32 *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_EFI64:
                if (!mb2_efi64) {
                    mb2_efi64 = (struct multiboot_tag_efi64 *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_SMBIOS:
                if (!mb2_smbios) {
                    mb2_smbios = (struct multiboot_tag_smbios *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_ACPI_OLD:
                if (!mb2_acpi_old) {
                    mb2_acpi_old = (struct multiboot_tag_old_acpi *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_ACPI_NEW:
                if (!mb2_acpi_new) {
                    mb2_acpi_new = (struct multiboot_tag_new_acpi *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_NETWORK:
                if (!mb2_network) {
                    mb2_network = (struct multiboot_tag_network *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_EFI_MMAP:
                if (!mb2_efi_mmap) {
                    mb2_efi_mmap = (struct multiboot_tag_efi_mmap *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_EFI_BS:
                /* EFI boot services - typically not needed after boot */
                break;
                
            case MULTIBOOT_TAG_TYPE_EFI32_IH:
                if (!mb2_efi32_ih) {
                    mb2_efi32_ih = (struct multiboot_tag_efi32_ih *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_EFI64_IH:
                if (!mb2_efi64_ih) {
                    mb2_efi64_ih = (struct multiboot_tag_efi64_ih *)tag;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
                if (!mb2_load_base_addr) {
                    mb2_load_base_addr = (struct multiboot_tag_load_base_addr *)tag;
                }
                break;
                
            default:
                /* Unknown tag, skip */
                break;
        }
        
        /* Move to next tag (tags are 8-byte aligned) */
        tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
    }
}