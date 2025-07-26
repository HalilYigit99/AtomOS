/* AtomOS Linker Script */
ENTRY(_start)

SECTIONS
{
    . = 1M;

    .boot :
    {
        /* Multiboot header ilk sırada olmalı */
        *(.multiboot)
    }

    .text :
    {
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(COMMON)
        *(.bss)
    }
}