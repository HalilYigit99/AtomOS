#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Multiboot2 magic numbers */
#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6
#define MULTIBOOT2_BOOTLOADER_MAGIC     0x36d76289

/* Multiboot2 tag types */
#define MULTIBOOT_TAG_TYPE_END                  0
#define MULTIBOOT_TAG_TYPE_CMDLINE              1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME     2
#define MULTIBOOT_TAG_TYPE_MODULE               3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO        4
#define MULTIBOOT_TAG_TYPE_BOOTDEV              5
#define MULTIBOOT_TAG_TYPE_MMAP                 6
#define MULTIBOOT_TAG_TYPE_VBE                  7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER          8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS         9
#define MULTIBOOT_TAG_TYPE_APM                  10
#define MULTIBOOT_TAG_TYPE_EFI32                11
#define MULTIBOOT_TAG_TYPE_EFI64                12
#define MULTIBOOT_TAG_TYPE_SMBIOS               13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD             14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW             15
#define MULTIBOOT_TAG_TYPE_NETWORK              16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP             17
#define MULTIBOOT_TAG_TYPE_EFI_BS               18
#define MULTIBOOT_TAG_TYPE_EFI32_IH             19
#define MULTIBOOT_TAG_TYPE_EFI64_IH             20
#define MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR       21

/* Multiboot2 structures */
struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_tag_string {
    uint32_t type;
    uint32_t size;
    char string[0];
};

struct multiboot_tag_module {
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    char cmdline[0];
};

struct multiboot_tag_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
};

struct multiboot_tag_bootdev {
    uint32_t type;
    uint32_t size;
    uint32_t biosdev;
    uint32_t slice;
    uint32_t part;
};

struct multiboot_mmap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t reserved;
};

struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot_mmap_entry entries[0];
};

struct multiboot_tag_framebuffer {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
};

struct multiboot_tag_elf_sections {
    uint32_t type;
    uint32_t size;
    uint32_t num;
    uint32_t entsize;
    uint32_t shndx;
    char sections[0];
};

struct multiboot_tag_apm {
    uint32_t type;
    uint32_t size;
    uint16_t version;
    uint16_t cseg;
    uint32_t offset;
    uint16_t cseg_16;
    uint16_t dseg;
    uint16_t flags;
    uint16_t cseg_len;
    uint16_t cseg_16_len;
    uint16_t dseg_len;
};

struct multiboot_tag_efi32 {
    uint32_t type;
    uint32_t size;
    uint32_t pointer;
};

struct multiboot_tag_efi64 {
    uint32_t type;
    uint32_t size;
    uint64_t pointer;
};

struct multiboot_tag_smbios {
    uint32_t type;
    uint32_t size;
    uint8_t major;
    uint8_t minor;
    uint8_t reserved[6];
    uint8_t tables[0];
};

struct multiboot_tag_old_acpi {
    uint32_t type;
    uint32_t size;
    uint8_t rsdp[0];
};

struct multiboot_tag_new_acpi {
    uint32_t type;
    uint32_t size;
    uint8_t rsdp[0];
};

struct multiboot_tag_network {
    uint32_t type;
    uint32_t size;
    uint8_t dhcpack[0];
};

struct multiboot_tag_efi_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t descr_size;
    uint32_t descr_vers;
    uint8_t efi_mmap[0];
};

struct multiboot_tag_efi32_ih {
    uint32_t type;
    uint32_t size;
    uint32_t pointer;
};

struct multiboot_tag_efi64_ih {
    uint32_t type;
    uint32_t size;
    uint64_t pointer;
};

struct multiboot_tag_load_base_addr {
    uint32_t type;
    uint32_t size;
    uint32_t load_base_addr;
};

/* External tag pointers */
extern struct multiboot_tag_string *mb2_cmdline;
extern struct multiboot_tag_string *mb2_bootloader_name;
extern struct multiboot_tag_module *mb2_module;
extern struct multiboot_tag_basic_meminfo *mb2_basic_meminfo;
extern struct multiboot_tag_bootdev *mb2_bootdev;
extern struct multiboot_tag_mmap *mb2_mmap;
extern struct multiboot_tag_framebuffer *mb2_framebuffer;
extern struct multiboot_tag_elf_sections *mb2_elf_sections;
extern struct multiboot_tag_apm *mb2_apm;
extern struct multiboot_tag_efi32 *mb2_efi32;
extern struct multiboot_tag_efi64 *mb2_efi64;
extern struct multiboot_tag_smbios *mb2_smbios;
extern struct multiboot_tag_old_acpi *mb2_acpi_old;
extern struct multiboot_tag_new_acpi *mb2_acpi_new;
extern struct multiboot_tag_network *mb2_network;
extern struct multiboot_tag_efi_mmap *mb2_efi_mmap;
extern struct multiboot_tag_efi32_ih *mb2_efi32_ih;
extern struct multiboot_tag_efi64_ih *mb2_efi64_ih;
extern struct multiboot_tag_load_base_addr *mb2_load_base_addr;

/* Function prototypes */
void multiboot2_parse(uintptr_t mb2_info_addr);

#ifdef __cplusplus
}
#endif