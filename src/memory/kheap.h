#ifndef KHEAP_H
#define KHEAP_H

#include <stdint.h>
#include "../misc/printf.h"
#include "../misc/mem.h"
#include "pmm.h"

#define KHEAP_VIRT_START 0xd0000000
#define KHEAP_INITIAL_SIZE (4 * 1024 * 1024)

typedef struct k_heap_block {
    uint32_t addr;
    uint32_t size;
    uint8_t free;
    struct k_heap_block *prev;
    struct k_heap_block *next;
} k_heap_block_t;

extern uint32_t kheap_start;
extern uint32_t kheap_end;
extern uint32_t kheap_size;
extern k_heap_block_t *head;
extern k_heap_block_t *free_head;

void kheap_init();
void *kmalloc(uint32_t size);
void *kalloc_pages(uint32_t num_pages);
void kfree(void *addr);

#endif