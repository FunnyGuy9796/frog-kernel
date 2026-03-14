#include "paging.h"

extern uint32_t page_directory[1024];

uint32_t kernel_pd_phys = 0;

void paging_init() {
    kernel_pd_phys = (uint32_t)page_directory - KERNEL_OFFSET;

    page_directory[RECURSIVE_SLOT] = kernel_pd_phys | PAGE_PRESENT | PAGE_RW;

    __asm__ volatile ("mov %%cr3, %%eax; mov %%eax, %%cr3" ::: "eax", "memory");
}

void map_page(uint32_t phys, uint32_t virt, uint32_t flags) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3ff;
    uint32_t *page_directory = (uint32_t *)PAGE_DIR_VIRT;
    uint32_t *pt_virt = (uint32_t *)(RECURSIVE_BASE + pd_index * 0x1000);

    if (!(page_directory[pd_index] & PAGE_PRESENT)) {
        uint32_t pt_phys = pmm_alloc_frame();

        if (!pt_phys)
            panic(NULL, "map_page() -> failed to allocate page table");
        
        uint32_t pd_flags = PAGE_PRESENT | PAGE_RW;

        if (flags & PAGE_USER)
            pd_flags |= PAGE_USER;
        
        page_directory[pd_index] = pt_phys | pd_flags;

        memset(pt_virt, 0, PAGE_SIZE);
    }

    pt_virt[pt_index] = (phys & ~0xfff) | (flags & 0xfff) | PAGE_PRESENT;

    __asm__ volatile ("invlpg (%0)" :: "r"(virt) : "memory");
}

void unmap_page(uint32_t virt) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3ff;
    uint32_t *page_directory = (uint32_t *)PAGE_DIR_VIRT;
    uint32_t *pt_virt = (uint32_t *)(RECURSIVE_BASE + pd_index * 0x1000);

    if (!(page_directory[pd_index] & PAGE_PRESENT))
        return;
    
    if (!(pt_virt[pt_index] & PAGE_PRESENT))
        return;
    
    pt_virt[pt_index] = 0;

    __asm__ volatile ("invlpg (%0)" :: "r"(virt) : "memory");

    int empty = 1;

    for (int i = 0; i < 1024; i++) {
        if (pt_virt[i] & PAGE_PRESENT) {
            empty = 0;

            break;
        }
    }

    if (empty) {
        uint32_t pt_phys = page_directory[pd_index] & ~0xfff;

        page_directory[pd_index] = 0;

        __asm__ volatile ("invlpg (%0)" :: "r"(virt) : "memory");

        pmm_free_frame(pt_phys);
    }
}