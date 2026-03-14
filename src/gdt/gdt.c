#include "gdt.h"

gdt_entry_t gdt[GDT_SIZE];
gdt_ptr_t gdt_ptr;
tss_t kernel_tss;

static void gdt_set_tss(int i, uint32_t base, uint32_t limit) {
    gdt[i].base_low = base & 0xffff;
    gdt[i].base_mid = (base >> 16) & 0xff;
    gdt[i].base_high = (base >> 24) & 0xff;

    gdt[i].limit_low = limit & 0xffff;
    gdt[i].granularity = 0x00;

    gdt[i].access = 0x89;
}

void gdt_set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[i].base_low = base & 0xffff;
    gdt[i].base_mid = (base >> 16) & 0xff;
    gdt[i].base_high = (base >> 24) & 0xff;

    gdt[i].limit_low = limit & 0xffff;
    gdt[i].granularity = ((limit >> 16) & 0x0f) | (gran & 0xf0);

    gdt[i].access = access;
}

void gdt_init() {
    gdt_set_entry(0, 0, 0x00000000, 0x00, 0x00);
    gdt_set_entry(1, 0, 0xffffffff, 0x9a, 0xcf);
    gdt_set_entry(2, 0, 0xffffffff, 0x92, 0xcf);
    gdt_set_entry(3, 0, 0xffffffff, 0xfa, 0xcf);
    gdt_set_entry(4, 0, 0xffffffff, 0xf2, 0xcf);

    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint32_t)&gdt;

    gdt_flush(&gdt_ptr);

    tss_init();
}

void tss_init() {
    memset(&kernel_tss, 0, sizeof(tss_t));

    kernel_tss.ss0 = SEG_KDATA;
    kernel_tss.esp0 = 0;
    kernel_tss.iomap_base = sizeof(tss_t);

    gdt_set_tss(5, (uint32_t)&kernel_tss, sizeof(tss_t) - 1);

    __asm__ volatile ("ltr %%ax" :: "a"(SEG_TSS));
}

void tss_set_kernel_stack(uint32_t esp0) {
    kernel_tss.esp0 = esp0;
}