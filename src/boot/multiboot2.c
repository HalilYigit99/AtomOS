#include <boot/multiboot2.h>
#include <stddef.h>
#include <string.h>
#include <stream/OutputStream.h>

/* Static local copies of tags (not in header) */
static struct multiboot_tag_string mb2_cmdline_copy;
static struct multiboot_tag_string mb2_bootloader_name_copy;
static struct multiboot_tag_module mb2_module_copy;
static struct multiboot_tag_basic_meminfo mb2_basic_meminfo_copy;
static struct multiboot_tag_bootdev mb2_bootdev_copy;
static struct multiboot_tag_mmap mb2_mmap_copy;
static struct multiboot_tag_framebuffer mb2_framebuffer_copy;
static struct multiboot_tag_elf_sections mb2_elf_sections_copy;
static struct multiboot_tag_apm mb2_apm_copy;
static struct multiboot_tag_efi32 mb2_efi32_copy;
static struct multiboot_tag_efi64 mb2_efi64_copy;
static struct multiboot_tag_smbios mb2_smbios_copy;
static struct multiboot_tag_old_acpi mb2_acpi_old_copy;
static struct multiboot_tag_new_acpi mb2_acpi_new_copy;
static struct multiboot_tag_network mb2_network_copy;
static struct multiboot_tag_efi_mmap mb2_efi_mmap_copy;
static struct multiboot_tag_efi32_ih mb2_efi32_ih_copy;
static struct multiboot_tag_efi64_ih mb2_efi64_ih_copy;
static struct multiboot_tag_load_base_addr mb2_load_base_addr_copy;

/* Dynamic buffers for variable-length data */
static char cmdline_buffer[256];
static char bootloader_name_buffer[256];
static char module_cmdline_buffer[256];
static struct multiboot_mmap_entry mmap_entries_buffer[64];
static uint8_t smbios_tables_buffer[1024];
static uint8_t acpi_old_rsdp_buffer[1024];
static uint8_t acpi_new_rsdp_buffer[1024];
static uint8_t network_dhcpack_buffer[1024];
static uint8_t efi_mmap_buffer[2048];
static char elf_sections_buffer[4096];

/* Global tag pointers - will point to local copies */
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

extern uint32_t mb2_tagptr; // Pointer to the multiboot2 tags

void multiboot2_parse() {
    struct multiboot_tag *tag;
    
    /* Skip the first 8 bytes (total_size and reserved) */
    tag = (struct multiboot_tag *)(mb2_tagptr + 8);
    
    /* Parse all tags */
    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                if (!mb2_cmdline) {
                    struct multiboot_tag_string *str_tag = (struct multiboot_tag_string *)tag;
                    
                    /* Copy the fixed part */
                    memcpy(&mb2_cmdline_copy, str_tag, sizeof(struct multiboot_tag_string));
                    
                    /* Copy the string data */
                    size_t str_len = str_tag->size - sizeof(struct multiboot_tag_string);
                    if (str_len > sizeof(cmdline_buffer) - 1) {
                        str_len = sizeof(cmdline_buffer) - 1;
                    }
                    memcpy(cmdline_buffer, str_tag->string, str_len);
                    cmdline_buffer[str_len] = '\0';
                    
                    /* Point to our copy */
                    mb2_cmdline = &mb2_cmdline_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                if (!mb2_bootloader_name) {
                    struct multiboot_tag_string *str_tag = (struct multiboot_tag_string *)tag;
                    
                    /* Copy the fixed part */
                    memcpy(&mb2_bootloader_name_copy, str_tag, sizeof(struct multiboot_tag_string));
                    
                    /* Copy the string data */
                    size_t str_len = str_tag->size - sizeof(struct multiboot_tag_string);
                    if (str_len > sizeof(bootloader_name_buffer) - 1) {
                        str_len = sizeof(bootloader_name_buffer) - 1;
                    }
                    memcpy(bootloader_name_buffer, str_tag->string, str_len);
                    bootloader_name_buffer[str_len] = '\0';
                    
                    /* Point to our copy */
                    mb2_bootloader_name = &mb2_bootloader_name_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_MODULE:
                if (!mb2_module) {
                    struct multiboot_tag_module *mod_tag = (struct multiboot_tag_module *)tag;
                    
                    /* Copy the fixed part */
                    memcpy(&mb2_module_copy, mod_tag, sizeof(struct multiboot_tag_module));
                    
                    /* Copy the cmdline data */
                    size_t cmdline_len = mod_tag->size - sizeof(struct multiboot_tag_module);
                    if (cmdline_len > sizeof(module_cmdline_buffer) - 1) {
                        cmdline_len = sizeof(module_cmdline_buffer) - 1;
                    }
                    memcpy(module_cmdline_buffer, mod_tag->cmdline, cmdline_len);
                    module_cmdline_buffer[cmdline_len] = '\0';
                    
                    /* Point to our copy */
                    mb2_module = &mb2_module_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                if (!mb2_basic_meminfo) {
                    /* Fixed size structure, simple copy */
                    memcpy(&mb2_basic_meminfo_copy, tag, sizeof(struct multiboot_tag_basic_meminfo));
                    mb2_basic_meminfo = &mb2_basic_meminfo_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_BOOTDEV:
                if (!mb2_bootdev) {
                    /* Fixed size structure, simple copy */
                    memcpy(&mb2_bootdev_copy, tag, sizeof(struct multiboot_tag_bootdev));
                    mb2_bootdev = &mb2_bootdev_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_MMAP:
                if (!mb2_mmap) {
                    struct multiboot_tag_mmap *mmap_tag = (struct multiboot_tag_mmap *)tag;
                    
                    /* Copy the fixed part */
                    memcpy(&mb2_mmap_copy, mmap_tag, sizeof(struct multiboot_tag_mmap));
                    
                    /* Copy memory map entries */
                    size_t entries_size = mmap_tag->size - sizeof(struct multiboot_tag_mmap);
                    size_t max_entries_size = sizeof(mmap_entries_buffer);
                    if (entries_size > max_entries_size) {
                        entries_size = max_entries_size;
                    }
                    memcpy(mmap_entries_buffer, mmap_tag->entries, entries_size);
                    
                    /* Point to our copy */
                    mb2_mmap = &mb2_mmap_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
                if (!mb2_framebuffer) {
                    /* Copy the entire framebuffer structure */
                    memcpy(&mb2_framebuffer_copy, tag, sizeof(struct multiboot_tag_framebuffer));
                    mb2_framebuffer = &mb2_framebuffer_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
                if (!mb2_elf_sections) {
                    struct multiboot_tag_elf_sections *elf_tag = (struct multiboot_tag_elf_sections *)tag;
                    
                    /* Copy the fixed part */
                    memcpy(&mb2_elf_sections_copy, elf_tag, sizeof(struct multiboot_tag_elf_sections));
                    
                    /* Copy sections data */
                    size_t sections_size = elf_tag->size - sizeof(struct multiboot_tag_elf_sections);
                    if (sections_size > sizeof(elf_sections_buffer)) {
                        sections_size = sizeof(elf_sections_buffer);
                    }
                    memcpy(elf_sections_buffer, elf_tag->sections, sections_size);
                    
                    /* Point to our copy */
                    mb2_elf_sections = &mb2_elf_sections_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_APM:
                if (!mb2_apm) {
                    /* Fixed size structure, simple copy */
                    memcpy(&mb2_apm_copy, tag, sizeof(struct multiboot_tag_apm));
                    mb2_apm = &mb2_apm_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_EFI32:
                if (!mb2_efi32) {
                    /* Fixed size structure, simple copy */
                    memcpy(&mb2_efi32_copy, tag, sizeof(struct multiboot_tag_efi32));
                    mb2_efi32 = &mb2_efi32_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_EFI64:
                if (!mb2_efi64) {
                    /* Fixed size structure, simple copy */
                    memcpy(&mb2_efi64_copy, tag, sizeof(struct multiboot_tag_efi64));
                    mb2_efi64 = &mb2_efi64_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_SMBIOS:
                if (!mb2_smbios) {
                    struct multiboot_tag_smbios *smbios_tag = (struct multiboot_tag_smbios *)tag;
                    
                    /* Copy the fixed part */
                    memcpy(&mb2_smbios_copy, smbios_tag, sizeof(struct multiboot_tag_smbios));
                    
                    /* Copy tables data */
                    size_t tables_size = smbios_tag->size - sizeof(struct multiboot_tag_smbios);
                    if (tables_size > sizeof(smbios_tables_buffer)) {
                        tables_size = sizeof(smbios_tables_buffer);
                    }
                    memcpy(smbios_tables_buffer, smbios_tag->tables, tables_size);
                    
                    /* Point to our copy */
                    mb2_smbios = &mb2_smbios_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_ACPI_OLD:
                if (!mb2_acpi_old) {
                    struct multiboot_tag_old_acpi *acpi_tag = (struct multiboot_tag_old_acpi *)tag;
                    
                    /* Copy the fixed part */
                    memcpy(&mb2_acpi_old_copy, acpi_tag, sizeof(struct multiboot_tag_old_acpi));
                    
                    /* Copy RSDP data */
                    size_t rsdp_size = acpi_tag->size - sizeof(struct multiboot_tag_old_acpi);
                    if (rsdp_size > sizeof(acpi_old_rsdp_buffer)) {
                        rsdp_size = sizeof(acpi_old_rsdp_buffer);
                    }
                    memcpy(acpi_old_rsdp_buffer, acpi_tag->rsdp, rsdp_size);
                    
                    /* Point to our copy */
                    mb2_acpi_old = &mb2_acpi_old_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_ACPI_NEW:
                if (!mb2_acpi_new) {
                    struct multiboot_tag_new_acpi *acpi_tag = (struct multiboot_tag_new_acpi *)tag;
                    
                    /* Copy the fixed part */
                    memcpy(&mb2_acpi_new_copy, acpi_tag, sizeof(struct multiboot_tag_new_acpi));
                    
                    /* Copy RSDP data */
                    size_t rsdp_size = acpi_tag->size - sizeof(struct multiboot_tag_new_acpi);
                    if (rsdp_size > sizeof(acpi_new_rsdp_buffer)) {
                        rsdp_size = sizeof(acpi_new_rsdp_buffer);
                    }
                    memcpy(acpi_new_rsdp_buffer, acpi_tag->rsdp, rsdp_size);
                    
                    /* Point to our copy */
                    mb2_acpi_new = &mb2_acpi_new_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_NETWORK:
                if (!mb2_network) {
                    struct multiboot_tag_network *net_tag = (struct multiboot_tag_network *)tag;
                    
                    /* Copy the fixed part */
                    memcpy(&mb2_network_copy, net_tag, sizeof(struct multiboot_tag_network));
                    
                    /* Copy DHCP ACK data */
                    size_t dhcpack_size = net_tag->size - sizeof(struct multiboot_tag_network);
                    if (dhcpack_size > sizeof(network_dhcpack_buffer)) {
                        dhcpack_size = sizeof(network_dhcpack_buffer);
                    }
                    memcpy(network_dhcpack_buffer, net_tag->dhcpack, dhcpack_size);
                    
                    /* Point to our copy */
                    mb2_network = &mb2_network_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_EFI_MMAP:
                if (!mb2_efi_mmap) {
                    struct multiboot_tag_efi_mmap *efi_mmap_tag = (struct multiboot_tag_efi_mmap *)tag;
                    
                    /* Copy the fixed part */
                    memcpy(&mb2_efi_mmap_copy, efi_mmap_tag, sizeof(struct multiboot_tag_efi_mmap));
                    
                    /* Copy EFI memory map data */
                    size_t efi_mmap_size = efi_mmap_tag->size - sizeof(struct multiboot_tag_efi_mmap);
                    if (efi_mmap_size > sizeof(efi_mmap_buffer)) {
                        efi_mmap_size = sizeof(efi_mmap_buffer);
                    }
                    memcpy(efi_mmap_buffer, efi_mmap_tag->efi_mmap, efi_mmap_size);
                    
                    /* Point to our copy */
                    mb2_efi_mmap = &mb2_efi_mmap_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_EFI_BS:
                /* EFI boot services - typically not needed after boot */
                break;
                
            case MULTIBOOT_TAG_TYPE_EFI32_IH:
                if (!mb2_efi32_ih) {
                    /* Fixed size structure, simple copy */
                    memcpy(&mb2_efi32_ih_copy, tag, sizeof(struct multiboot_tag_efi32_ih));
                    mb2_efi32_ih = &mb2_efi32_ih_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_EFI64_IH:
                if (!mb2_efi64_ih) {
                    /* Fixed size structure, simple copy */
                    memcpy(&mb2_efi64_ih_copy, tag, sizeof(struct multiboot_tag_efi64_ih));
                    mb2_efi64_ih = &mb2_efi64_ih_copy;
                }
                break;
                
            case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
                if (!mb2_load_base_addr) {
                    /* Fixed size structure, simple copy */
                    memcpy(&mb2_load_base_addr_copy, tag, sizeof(struct multiboot_tag_load_base_addr));
                    mb2_load_base_addr = &mb2_load_base_addr_copy;
                }
                break;
                
            default:
                /* Unknown tag, skip */
                break;
        }
        
        /* Move to next tag (tags are 8-byte aligned) */
        tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
    }

    if (
        mb2_framebuffer == NULL ||
        mb2_mmap == NULL ||
        mb2_basic_meminfo == NULL ||
        mb2_cmdline == NULL
    ) {

        // Set msg for textmode terminal
        char* msg = (char*)"Multiboot2 tags are missing required information!\n";

        char* txtBfr = (char*)0xB8000;

        char* msg_tmp = msg;

        while (*msg_tmp) {
            *txtBfr++ = *msg_tmp; // Write character
            *txtBfr++ = 0x1F; // White on black attribute
            msg_tmp++;
        }

        currentOutputStream->printf(msg);
    }

}