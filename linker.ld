OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

KERNEL_OFFSET = 0xc0000000;
KERNEL_PHYS_BASE = 0x00200000;

SECTIONS
{
    . = KERNEL_PHYS_BASE;

    .multiboot . : ALIGN(8) {
        KEEP(*(.multiboot))
    }

    .text.boot : ALIGN(4096) {
        *(.text.boot)
    }

    . = KERNEL_OFFSET + ALIGN(LOADADDR(.text.boot) + SIZEOF(.text.boot), 4096);

    kernel_virt_start = .;

    .text : AT(ADDR(.text) - KERNEL_OFFSET) ALIGN(4096)
    {
        *(.text*)
    }

    .rodata : AT(ADDR(.rodata) - KERNEL_OFFSET) ALIGN(4096)
    {
        *(.rodata*)
    }

    .data : AT(ADDR(.data) - KERNEL_OFFSET) ALIGN(4096)
    {
        *(.data*)
    }

    .bss : AT(ADDR(.bss) - KERNEL_OFFSET) ALIGN(4096)
    {
        *(COMMON)
        *(.bss*)
    }

    .stack : AT(ADDR(.stack) - KERNEL_OFFSET) ALIGN(4096)
    {
        *(.stack*)
    }

    kernel_virt_end = .;

    kernel_phys_start = kernel_virt_start - KERNEL_OFFSET;
    kernel_phys_end = kernel_virt_end - KERNEL_OFFSET;
}