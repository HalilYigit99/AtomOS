#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t present : 1;      // Page present in memory
    uint32_t rw : 1;           // Read/Write permission
    uint32_t user : 1;         // User/Supervisor mode
    uint32_t pwt : 1;          // Page-level write-through
    uint32_t pcd : 1;          // Page-level cache disable
    uint32_t accessed : 1;     // Accessed flag
    uint32_t dirty : 1;        // Dirty flag
    uint32_t pat : 1;          // Page Attribute Table
    uint32_t global : 1;       // Global page
    uint32_t available : 3;    // Available for software use
    uint32_t frame : 20;       // Frame address (physical page number)
} __attribute__((packed)) PageTableEntry;

typedef struct {
    uint32_t present : 1;      // Page directory entry present
    uint32_t rw : 1;           // Read/Write permission
    uint32_t user : 1;         // User/Supervisor mode
    uint32_t pwt : 1;          // Page-level write-through
    uint32_t pcd : 1;          // Page-level cache disable
    uint32_t accessed : 1;     // Accessed flag
    uint32_t reserved : 1;     // Reserved bit (must be zero)
    uint32_t page_size : 1;    // Page size (0 for 4KB, 1 for 4MB)
    uint32_t global : 1;       // Global page
    uint32_t available : 3;    // Available for software use
    uint32_t frame : 20;       // Frame address (physical page number)
} __attribute__((packed)) PageDirectoryEntry;

PageTableEntry intel86_page_table[1024][1024] __attribute__((aligned(4096)));
PageDirectoryEntry intel86_page_directory[1024] __attribute__((aligned(4096)));

void intel86_paging_init() {
    // Map all the pages identically for simplicity
    for (size_t i = 0; i < 1024; i++) {
        for (size_t j = 0; j < 1024; j++) {
            intel86_page_table[i][j].present = 1;
            intel86_page_table[i][j].rw = 1;
            intel86_page_table[i][j].user = 1;
            intel86_page_table[i][j].frame = (i * 1024 + j); // Identity mapping: virtual = physical
            intel86_page_table[i][j].available = 0;
            intel86_page_table[i][j].accessed = 0;
            intel86_page_table[i][j].dirty = 0;
            intel86_page_table[i][j].pwt = 0;
            intel86_page_table[i][j].pcd = 0;
            intel86_page_table[i][j].pat = 0;
            intel86_page_table[i][j].global = 0;
        }
        intel86_page_directory[i].present = 1;
        intel86_page_directory[i].rw = 1;
        intel86_page_directory[i].user = 1;
        intel86_page_directory[i].frame = ((uintptr_t)intel86_page_table[i]) >> 12; // Physical address >> 12
        intel86_page_directory[i].available = 0;
        intel86_page_directory[i].accessed = 0;
        intel86_page_directory[i].reserved = 0;
        intel86_page_directory[i].page_size = 0; // 4KB pages
        intel86_page_directory[i].pwt = 0;
        intel86_page_directory[i].pcd = 0;
        intel86_page_directory[i].global = 0;
    }

    // Load the page directory into the CR3 register
    uint32_t page_dir_addr = (uint32_t)(uintptr_t)intel86_page_directory;
    asm volatile("movl %0, %%cr3" : : "r"(page_dir_addr) : "memory");
    
    // Enable paging by setting the PG bit in CR0
    uint32_t cr0;
    asm volatile("movl %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set the PG bit (bit 31)
    asm volatile("movl %0, %%cr0" : : "r"(cr0) : "memory");

    // Flush the TLB (Translation Lookaside Buffer) - reload CR3
    asm volatile(
        "movl %%cr3, %%eax\n\t"
        "movl %%eax, %%cr3"
        :
        :
        : "eax", "memory"
    );

}
