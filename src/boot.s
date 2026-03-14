.set KERNEL_OFFSET, 0xc0000000
.set KERNEL_PHYS_BASE, 0x00200000

.set MB2_MAGIC, 0xe85250d6
.set MB2_ARCH, 0
.set MB2_LENGTH, (header_end - header_start)
.set MB2_CHECKSUM, -(MB2_MAGIC + MB2_ARCH + MB2_LENGTH)

.section .multiboot, "a"
.align 8
header_start:
    .long  MB2_MAGIC
    .long  MB2_ARCH
    .long  MB2_LENGTH
    .long  MB2_CHECKSUM

    # Framebuffer tag
    .align 8
    .word 5
    .word 0
    .long 20
    .long 1024
    .long 768
    .long 32

    # End tag
    .align 8
    .word  0
    .word  0
    .long  8
header_end:

.section .data
.align 4096
.global page_directory
page_directory:
    .fill 1024, 4, 0

.align 4096
.global page_table_low
page_table_low:
    .fill 1024, 4, 0

.section .stack, "aw", @nobits
.align 16
stack_bottom:
    .skip 16384
stack_top:

.section .text.boot, "ax"
.global _start
.type _start, @function
_start:
    cli

    movl %ebx, (mb2_info_ptr - KERNEL_OFFSET)
    movl $(stack_top - KERNEL_OFFSET), %esp

    movl $0, %ecx

.fill_page_table:
    movl %ecx, %eax

    shll $12, %eax
    orl $0x3, %eax

    movl %eax, (page_table_low - KERNEL_OFFSET)(, %ecx, 4)
    incl %ecx

    cmpl $1024, %ecx
    jl .fill_page_table

    movl $(page_table_low - KERNEL_OFFSET), %eax
    orl $0x3, %eax

    movl %eax, (page_directory - KERNEL_OFFSET)
    movl %eax, (page_directory - KERNEL_OFFSET + 768 * 4)

    movl $(page_directory - KERNEL_OFFSET), %eax
    movl %eax, %cr3

    lgdt (boot_gdt_descriptor)

    movl %cr0, %eax
    orl $0x80000001, %eax
    movl %eax, %cr0

    movl $higher_half_entry, %eax
    pushl $0x08
    pushl %eax

    ljmp *(%esp)

.size _start, . - _start

.align 8
boot_gdt_start:
    .quad 0x0000000000000000

    .quad 0x00CF9A000000FFFF

    .quad 0x00CF92000000FFFF
boot_gdt_end:

boot_gdt_descriptor:
    .word (boot_gdt_end - boot_gdt_start - 1)
    .long boot_gdt_start

.section .text
higher_half_entry:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    addl $6, %esp
    addl $KERNEL_OFFSET, %esp

    # FPU initialization
    mov %cr0, %eax
    and $0xfffffffb, %eax
    or $0x00000002, %eax
    mov %eax, %cr0
    fninit

    mov %cr4, %eax
    or $0x00000600, %eax
    mov %eax, %cr4

    pushl (mb2_info_ptr - KERNEL_OFFSET)
    pushl $0x36d76289
    call kmain

    cli
1:  hlt
    jmp 1b

.global mb2_info_ptr
mb2_info_ptr:
    .long 0