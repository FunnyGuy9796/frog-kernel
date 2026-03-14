#include "kheap.h"

uint32_t kheap_start = 0;
uint32_t kheap_end = 0;
uint32_t kheap_size = 0;
k_heap_block_t *head = NULL;
k_heap_block_t *free_head = NULL;

static uint32_t large_alloc_base = 0xe0000000;

void kheap_init() {
    kheap_start = KHEAP_VIRT_START;
    kheap_end = kheap_start + KHEAP_INITIAL_SIZE;
    kheap_size = KHEAP_INITIAL_SIZE;

    for (uint32_t virt = kheap_start; virt < kheap_end; virt += PAGE_SIZE) {
        uint32_t frame_phys = pmm_alloc_frame();

        if (!frame_phys)
            panic(NULL, "kheap_init() -> out of physical memory");
        
        map_page(frame_phys, virt, PAGE_PRESENT | PAGE_RW);
    }

    head = (k_heap_block_t *)kheap_start;
    free_head = head;

    head->addr = kheap_start + sizeof(k_heap_block_t);
    head->size = KHEAP_INITIAL_SIZE - sizeof(k_heap_block_t);
    head->free = 1;
    head->prev = NULL;
    head->next = NULL;
}

void *kmalloc(uint32_t size) {
    k_heap_block_t *curr = free_head;

    while (curr != NULL) {
        if (curr->free && curr->size >= size) {
            if (curr->size >= size + sizeof(k_heap_block_t) + 1) {
                k_heap_block_t *new_block = (k_heap_block_t *)(curr->addr + size);

                new_block->addr = curr->addr + size + sizeof(k_heap_block_t);
                new_block->size = curr->size - size - sizeof(k_heap_block_t);
                new_block->free = 1;
                new_block->prev = curr;
                new_block->next = curr->next;

                if (curr->next != NULL)
                    curr->next->prev = new_block;
                
                curr->next = new_block;
            }

            curr->size = size;
            curr->free = 0;

            if (curr == free_head) {
                free_head = curr->next;

                while (free_head != NULL && !free_head->free)
                    free_head = free_head->next;
            }

            return (void *)curr->addr;
        }

        curr = curr->next;
    }

    uint32_t frame_phys = pmm_alloc_frame();

    map_page(frame_phys, kheap_end, PAGE_PRESENT | PAGE_RW);

    k_heap_block_t *new_block = (k_heap_block_t *)kheap_end;

    new_block->addr = kheap_end + sizeof(k_heap_block_t);
    new_block->size = PAGE_SIZE - sizeof(k_heap_block_t);
    new_block->free = 1;
    new_block->prev = NULL;
    new_block->next = NULL;

    kheap_end += PAGE_SIZE;
    kheap_size += PAGE_SIZE;

    k_heap_block_t *tail = head;

    while (tail->next != NULL)
        tail = tail->next;
    
    new_block->prev = tail;
    tail->next = new_block;

    if (free_head == NULL)
        free_head = new_block;
    
    return kmalloc(size);
}

void *kalloc_pages(uint32_t num_pages) {
    uint32_t virt = large_alloc_base;

    for (uint32_t i = 0; i < num_pages; i++) {
        uint32_t phys = pmm_alloc_frame();

        if (!phys)
            panic(NULL, "kalloc_pages() -> out of physical memory");
        
        map_page(phys, large_alloc_base, PAGE_PRESENT | PAGE_RW);

        large_alloc_base += PAGE_SIZE;
    }

    return (void *)virt;
}

void kfree(void *addr) {
    if (addr == NULL)
        return;
    
    k_heap_block_t *block = (k_heap_block_t *)((uint32_t)addr - sizeof(k_heap_block_t));

    if (block->free)
        return;
    
    block->free = 1;

    if (block->next != NULL && block->next->free) {
        k_heap_block_t *next = block->next;

        block->size += sizeof(k_heap_block_t) + next->size;
        block->next = next->next;

        if (next->next != NULL)
            next->next->prev = block;
    }

    if (block->prev != NULL && block->prev->free) {
        k_heap_block_t *prev = block->prev;

        prev->size += sizeof(k_heap_block_t) + block->size;
        prev->next = block->next;

        if (block->next != NULL)
            block->next->prev = prev;
        
        block = prev;
    }

    if (free_head == NULL || block < free_head)
        free_head = block;
}