#ifndef MMAP_H
#define MMAP_H

#include "../multiboot.h"
#include "../misc/util.h"
#include "../misc/printf.h"
#include "../misc/panic.h"

#define MAX_MEM_REGIONS 32

typedef struct {
    uint32_t base;
    uint32_t length;
} mem_region_t;

extern uint32_t kstart;
extern uint32_t kend;
extern uint32_t ksize;
extern uint32_t kpstart;
extern uint32_t kpend;
extern mem_region_t usable_regions[];
extern size_t usable_region_count;
extern uint32_t total_memory;

void mmap_init(void *mb2_info);

#endif