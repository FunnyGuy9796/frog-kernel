#include "pmm.h"

uint32_t total_pages = 0;
uint32_t bitmap_addr = 0;
uint32_t bitmap_size = 0;
uint8_t *pmm_bitmap = 0;

static inline uint32_t addr_to_page(uint32_t addr) {
    return addr / PAGE_SIZE;
}

void pmm_init(mb2_tag_framebuffer_t *fb_tag) {
    total_pages = total_memory / PAGE_SIZE;
    bitmap_addr = (kend + 0xfff) & ~0xfff;
    bitmap_size = (total_pages + 7) / 8;
    pmm_bitmap = (uint8_t *)bitmap_addr;

    memset(pmm_bitmap, 0xff, bitmap_size);

    for (size_t i = 0; i < usable_region_count; i++) {
        uint64_t base = usable_regions[i].base;
        uint64_t len = usable_regions[i].length;

        pmm_mark_free((uint32_t)base, (uint32_t)len);
    }

    uint32_t start_page = kpstart & ~0xfff;
    uint32_t end_page = (kpend + 0xfff) & ~0xfff;

    pmm_mark_used(0x00000000, 0x100000);
    pmm_mark_used(start_page, end_page - start_page);

    uint32_t bitmap_phys = bitmap_addr - KERNEL_OFFSET;
    uint32_t bitmap_start_page = bitmap_phys & ~0xfff;
    uint32_t bitmap_end_page = (bitmap_phys + bitmap_size + 0xfff) & ~0xfff;

    uint32_t fb_phys = (uint32_t)(uintptr_t)&fb - KERNEL_OFFSET;

    pmm_mark_used(bitmap_start_page, bitmap_end_page - bitmap_start_page);
    pmm_mark_used(fb_tag->framebuffer_addr, fb_tag->framebuffer_pitch * fb_tag->framebuffer_height);
    pmm_mark_used(fb_phys, sizeof(fb));
}

void pmm_mark_used(uint32_t addr, uint32_t len) {
    uint32_t page = addr_to_page(addr);
    uint32_t count = (len + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < count; i++)
        BIT_SET(pmm_bitmap, page + i);
}

void pmm_mark_free(uint32_t addr, uint32_t len) {
    uint32_t page = addr_to_page(addr);
    uint32_t count = (len + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < count; i++)
        BIT_CLEAR(pmm_bitmap, page + i);
}

int pmm_is_free(uint32_t addr) {
    return !BIT_TEST(pmm_bitmap, addr_to_page(addr));
}

uint32_t pmm_alloc_frame() {
    for (uint32_t i = 1; i < total_pages; i++) {
        if (!BIT_TEST(pmm_bitmap, i)) {
            BIT_SET(pmm_bitmap, i);

            return i * PAGE_SIZE;
        }
    }

    panic(NULL, "pmm_alloc_frame() -> out of physical memory");

    return 0;
}

void pmm_free_frame(uint32_t addr) {
    if (addr == 0)
        panic(NULL, "pmm_free_frame() -> attempt to free NULL page");
    
    if (addr & 0xfff)
        panic(NULL, "pmm_free_frame() -> address 0x%08x is not page-aligned", addr);
    
    uint32_t page = addr_to_page(addr);

    if (page >= total_pages)
        panic(NULL, "pmm_free_frame() -> address 0x%08x is out of range", addr);
    
    if (BIT_TEST(pmm_bitmap, page))
        panic(NULL, "pmm_free_frame() -> double free at 0x%08x", addr);

    BIT_CLEAR(pmm_bitmap, page);
}