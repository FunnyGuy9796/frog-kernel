#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include "../misc/printf.h"
#include "../misc/mem.h"
#include "mmap.h"

#define KERNEL_OFFSET 0xc0000000

#define PAGE_SIZE 0x1000
#define BIT_SET(map, bit) ((map)[(bit) / 8] |= (1 << ((bit) % 8)))
#define BIT_CLEAR(map, bit) ((map)[(bit) / 8] &= ~(1 << ((bit) % 8)))
#define BIT_TEST(map, bit) ((map)[(bit) / 8] & (1 << ((bit) % 8)))

extern uint32_t total_pages;
extern uint32_t bitmap_addr;
extern uint32_t bitmap_size;
extern uint8_t *pmm_bitmap;

void pmm_init(mb2_tag_framebuffer_t *fb_tag);
void pmm_mark_used(uint32_t addr, uint32_t len);
void pmm_mark_free(uint32_t addr, uint32_t len);
int pmm_is_free(uint32_t addr);
uint32_t pmm_alloc_frame();
void pmm_free_frame(uint32_t addr);

#endif