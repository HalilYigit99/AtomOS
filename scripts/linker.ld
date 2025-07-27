/* AtomOS Linker Script */
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
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

    .stack (NOLOAD) :
    {
        . = ALIGN(4);
        __stack_start = .;
        . += 0x100000; /* 1MB stack */
        __stack_end = .;
    }

    . = ALIGN(4);

    __kernel_end = .;

}