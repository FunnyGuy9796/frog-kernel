#ifndef GDT_H
#define GDT_H

#include <stdint.h>
#include "../misc/mem.h"

#define GDT_SIZE 6

#define SEG_KCODE 0x08
#define SEG_KDATA 0x10
#define SEG_UCODE 0x1b
#define SEG_UDATA 0x23
#define SEG_TSS 0x28

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip, eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) tss_t;

extern gdt_entry_t gdt[GDT_SIZE];
extern gdt_ptr_t gdt_ptr;
extern tss_t kernel_tss;

extern void gdt_flush();
void gdt_set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdt_init();
void tss_init();
void tss_set_kernel_stack(uint32_t esp0);

#endif