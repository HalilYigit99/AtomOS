/* AtomOS Linker Script */
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

/* Define memory segments with proper permissions */
PHDRS
{
    text PT_LOAD FLAGS(5);      /* R-X */
    data PT_LOAD FLAGS(6);      /* RW- */
}

SECTIONS
{
    . = 1M;

    .boot :
    {
        /* Multiboot header ilk sırada olmalı */
        *(.multiboot)
    } :text

    .text : ALIGN(4K)
    {
        *(.text)
    } :text

    .rodata : ALIGN(4K)
    {
        *(.rodata)
    } :text

    .data : ALIGN(4K)
    {
        *(.data)
    } :data

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    } :data

    .stack (NOLOAD) : ALIGN(4K)
    {
        . = ALIGN(4);
        __stack_start = .;
        . += 0x100000; /* 1MB stack */
        __stack_end = .;
    } :data

    . = ALIGN(4);

    __kernel_end = .;

    /* Discard sections that can cause RWX warnings */
    /DISCARD/ :
    {
        *(.note.gnu.property)
        *(.gnu.hash)
    }
}