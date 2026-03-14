#ifndef PAGING_H
#define PAGING_H

#include "pmm.h"
#include "../misc/util.h"

#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_USER 0x4
#define PAGE_PCD 0x10
#define PAGE_PWT 0x08

#define RECURSIVE_BASE 0xffc00000
#define PAGE_DIR_VIRT 0xfffff000
#define RECURSIVE_SLOT 1023

#define VIRT_TO_PHYS(addr) ((uint32_t)(addr) - KERNEL_VIRT_BASE)
#define PHYS_TO_VIRT(addr) ((uint32_t)(addr) + KERNEL_VIRT_BASE)

extern uint32_t kernel_pd_phys;

void paging_init();
void map_page(uint32_t phys, uint32_t virt, uint32_t flags);
void unmap_page(uint32_t virt);

#endif