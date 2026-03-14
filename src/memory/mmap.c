#include "mmap.h"

extern uint32_t kernel_phys_start;
extern uint32_t kernel_phys_end;
extern uint32_t kernel_virt_start;
extern uint32_t kernel_virt_end;

uint32_t kstart;
uint32_t kend;
uint32_t ksize;
uint32_t kpstart;
uint32_t kpend;
mem_region_t usable_regions[MAX_MEM_REGIONS];
size_t usable_region_count = 0;
uint32_t total_memory = 0;

void mmap_init(void *mb2_info) {
    kstart = (uint32_t)&kernel_virt_start;
    kend = (uint32_t)&kernel_virt_end;
    ksize = kend - kstart;

    kpstart = (uint32_t)&kernel_phys_start;
    kpend = (uint32_t)&kernel_phys_end;

    mb2_tag_t *tag = (mb2_tag_t *)((uint8_t *)mb2_info + 8);
    mb2_tag_mmap_t *mmap_tag = 0;

    while (tag->type != MB2_TAG_TYPE_END) {
        if (tag->type == MB2_TAG_TYPE_MMAP) {
            mmap_tag = (mb2_tag_mmap_t *)tag;

            break;
        }

        tag = (mb2_tag_t *)(((uint32_t)tag + tag->size + 7) & ~7);
    }

    if (!mmap_tag)
        panic(NULL, "mmap_init() -> no memory map tag found in multiboot2 info");
    
    uint8_t *entry_ptr = (uint8_t *)mmap_tag + sizeof(mb2_tag_mmap_t);
    uint8_t *mmap_end = (uint8_t *)mmap_tag + mmap_tag->size;

    while (entry_ptr < mmap_end) {
        mb2_mmap_entry_t *entry = (mb2_mmap_entry_t *)entry_ptr;

        if (entry->base_addr < 0x100000000ULL) {
            uint32_t base = (uint32_t)entry->base_addr;
            uint64_t top = entry->base_addr + entry->length;
            uint32_t length = (top > 0x100000000ULL) ? (uint32_t)(0x100000000ULL - entry->base_addr) : (uint32_t)entry->length;

            if (entry->type == 1 && usable_region_count < MAX_MEM_REGIONS) {
                usable_regions[usable_region_count].base = base;
                usable_regions[usable_region_count].length = length;
                usable_region_count++;
            }

            total_memory += length;
        }

        entry_ptr += mmap_tag->entry_size;
    }
}