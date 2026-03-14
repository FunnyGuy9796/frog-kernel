#include "uprocess.h"
#include "../vfs/vfs.h"

process_t *curr_process = NULL;

static process_t *process_list = NULL;
static uint32_t next_pid = 1;

static void process_list_add(process_t *proc) {
    proc->next = process_list;
    process_list = proc;
}

static void copy_to_phys(uint32_t phys, uint32_t offset_in_page, const uint8_t *src, uint32_t len) {
    uint32_t *kern_pd = (uint32_t *)PAGE_DIR_VIRT;
    
    kern_pd[TEMP_FRAME_SLOT] = phys | PAGE_PRESENT | PAGE_RW;

    __asm__ volatile ("invlpg (%0)" :: "r"(TEMP_FRAME_VIRT) : "memory");

    memcpy((void *)(TEMP_FRAME_VIRT + offset_in_page), src, len);

    kern_pd[TEMP_FRAME_SLOT] = 0;

    __asm__ volatile ("invlpg (%0)" :: "r"(TEMP_FRAME_VIRT) : "memory");
}

static void zero_phys(uint32_t phys, uint32_t offset_in_page, uint32_t len) {
    uint32_t *kern_pd = (uint32_t *)PAGE_DIR_VIRT;

    kern_pd[TEMP_FRAME_SLOT] = phys | PAGE_PRESENT | PAGE_RW;

    __asm__ volatile ("invlpg (%0)" :: "r"(TEMP_FRAME_VIRT) : "memory");

    memset((void *)(TEMP_FRAME_VIRT + offset_in_page), 0, len);

    kern_pd[TEMP_FRAME_SLOT] = 0;

    __asm__ volatile ("invlpg (%0)" :: "r"(TEMP_FRAME_VIRT) : "memory");
}

static void foreign_map_page(uint32_t pd_phys, uint32_t phys, uint32_t virt, uint32_t flags) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3ff;
    uint32_t *kern_pd = (uint32_t *)PAGE_DIR_VIRT;

    kern_pd[TEMP_PD_SLOT] = pd_phys | PAGE_PRESENT | PAGE_RW;

    __asm__ volatile ("invlpg (%0)" :: "r"(TEMP_PD_VIRT) : "memory");

    uint32_t *target_pd = (uint32_t *)TEMP_PD_VIRT;

    if (!(target_pd[pd_index] & PAGE_PRESENT)) {
        uint32_t pt_phys = pmm_alloc_frame();
        uint32_t pd_flags = PAGE_PRESENT | PAGE_RW;

        if (flags & PAGE_USER)
            pd_flags |= PAGE_USER;

        target_pd[pd_index] = pt_phys | pd_flags;
        kern_pd[TEMP_PT_SLOT] = pt_phys | PAGE_PRESENT | PAGE_RW;

        __asm__ volatile ("invlpg (%0)" :: "r"(TEMP_PT_VIRT) : "memory");

        memset((void *)TEMP_PT_VIRT, 0, PAGE_SIZE);

        kern_pd[TEMP_PT_SLOT] = 0;

        __asm__ volatile ("invlpg (%0)" :: "r"(TEMP_PT_VIRT) : "memory");
    }

    uint32_t pt_phys = target_pd[pd_index] & ~0xfff;

    kern_pd[TEMP_PT_SLOT] = pt_phys | PAGE_PRESENT | PAGE_RW;

    __asm__ volatile ("invlpg (%0)" :: "r"(TEMP_PT_VIRT) : "memory");

    uint32_t *target_pt = (uint32_t *)TEMP_PT_VIRT;

    target_pt[pt_index] = (phys & ~0xfff) | (flags & 0xfff) | PAGE_PRESENT;
    kern_pd[TEMP_PD_SLOT] = 0;
    kern_pd[TEMP_PT_SLOT] = 0;

    __asm__ volatile ("invlpg (%0)" :: "r"(TEMP_PD_VIRT) : "memory");
    __asm__ volatile ("invlpg (%0)" :: "r"(TEMP_PT_VIRT) : "memory");
}

process_t *uprocess_find(uint32_t pid) {
    process_t *proc = process_list;

    while (proc) {
        if (proc->pid == pid)
            return proc;
        
        proc = proc->next;
    }

    return NULL;
}

void uprocess_remove(uint32_t pid) {
    process_t **proc = &process_list;

    while (*proc) {
        if ((*proc)->pid == pid) {
            *proc = (*proc)->next;

            return;
        }

        proc = &(*proc)->next;
    }
}

uint32_t *uprocess_create_pagedir() {
    uint32_t pd_phys = pmm_alloc_frame();
    uint32_t *kern_pd = (uint32_t *)PAGE_DIR_VIRT;

    kern_pd[TEMP_PD_SLOT] = pd_phys | PAGE_PRESENT | PAGE_RW;;

    __asm__ volatile ("invlpg (%0)" :: "r"(TEMP_PD_VIRT) : "memory");

    uint32_t *new_pd = (uint32_t *)TEMP_PD_VIRT;

    memset(new_pd, 0, sizeof(uint32_t) * 768);

    for (int i = 768; i < 1024; i++)
        new_pd[i] = kern_pd[i];
    
    new_pd[RECURSIVE_SLOT] = pd_phys | PAGE_PRESENT | PAGE_RW;
    new_pd[TEMP_PD_SLOT] = 0;
    kern_pd[TEMP_PD_SLOT] = 0;

    __asm__ volatile ("invlpg (%0)" :: "r"(TEMP_PD_VIRT) : "memory");

    return (uint32_t *)pd_phys;
}

uint32_t uprocess_load(uint32_t *pd_phys, const uint8_t *data, uint32_t size, uint32_t *heap_start_out, uint32_t *stack_start_out, int argc, char **argv) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;
    uint32_t highest = 0;

    if (*(uint32_t *)ehdr->e_ident != ELF_MAGIC)
        panic(NULL, "uprocess_load() -> not a valid ELF binary");
    
    if (ehdr->e_type != ET_EXEC)
        panic(NULL, "uprocess_load() -> not an executable ELF");

    for (uint16_t i = 0; i < ehdr->e_phnum; i++) {
        Elf32_Phdr *phdr = (Elf32_Phdr *)(data + ehdr->e_phoff + i * ehdr->e_phentsize);

        if (phdr->p_type != PT_LOAD)
            continue;
        
        uint32_t seg_end = phdr->p_vaddr + phdr->p_memsz;

        if (seg_end > highest)
            highest = seg_end;
        
        if (phdr->p_vaddr >= KERNEL_VIRT_BASE)
            panic(NULL, "uprocess_load() -> segment overlaps kernel at 0x%08x", phdr->p_vaddr);
        
        if (phdr->p_vaddr + phdr->p_memsz > USER_STACK_TOP)
            panic(NULL, "uprocess_load() -> segment overlaps stack");
        
        uint32_t flags = PAGE_USER;

        if (phdr->p_flags & PF_W)
            flags |= PAGE_RW;
        
        uint32_t virt_start = phdr->p_vaddr & ~0xfff;
        uint32_t virt_end = phdr->p_vaddr + phdr->p_memsz;

        for (uint32_t virt = virt_start; virt < virt_end; virt += PAGE_SIZE) {
            uint32_t phys = pmm_alloc_frame();

            foreign_map_page((uint32_t)pd_phys, phys, virt, flags);

            uint32_t page_start = virt;
            uint32_t page_end = virt + PAGE_SIZE;
            uint32_t file_start = phdr->p_vaddr;
            uint32_t file_end = phdr->p_vaddr + phdr->p_filesz;

            if (page_end <= file_start || page_start >= file_end)
                zero_phys(phys, 0, PAGE_SIZE);
            else {
                uint32_t copy_start = (page_start < file_start) ? file_start : page_start;
                uint32_t copy_end = (page_end > file_end) ? file_end : page_end;
                uint32_t offset_in_page = copy_start - page_start;
                uint32_t offset_in_file = phdr->p_offset + (copy_start - file_start);
                uint32_t copy_len = copy_end - copy_start;

                zero_phys(phys, 0, PAGE_SIZE);
                copy_to_phys(phys, offset_in_page, data + offset_in_file, copy_len);
            }
        }
    }

    uint32_t stack_phys0 = pmm_alloc_frame();
    uint32_t stack_phys1 = pmm_alloc_frame();

    foreign_map_page((uint32_t)pd_phys, stack_phys0, USER_STACK_TOP - PAGE_SIZE * 2, PAGE_PRESENT | PAGE_RW | PAGE_USER);
    foreign_map_page((uint32_t)pd_phys, stack_phys1, USER_STACK_TOP - PAGE_SIZE, PAGE_PRESENT | PAGE_RW | PAGE_USER);

    zero_phys(stack_phys0, 0, PAGE_SIZE);
    zero_phys(stack_phys1, 0, PAGE_SIZE);

    uint8_t page_buf[PAGE_SIZE];

    memset(page_buf, 0, PAGE_SIZE);

    uint32_t buf_offset = PAGE_SIZE;
    uint32_t str_vaddrs[argc];

    for (int i = argc - 1; i >= 0; i--) {
        size_t len = strlen(argv[i]) + 1;

        buf_offset -= len;

        memcpy(page_buf + buf_offset, argv[i], len);

        str_vaddrs[i] = (USER_STACK_TOP - PAGE_SIZE) + buf_offset;
    }

    buf_offset &= ~3;
    buf_offset -= 4;
    *(uint32_t *)(page_buf + buf_offset) = 0;

    for (int i = argc - 1; i >= 0; i--) {
        buf_offset -= 4;
        *(uint32_t *)(page_buf + buf_offset) = str_vaddrs[i];
    }

    uint32_t argv_vaddr = (USER_STACK_TOP - PAGE_SIZE) + buf_offset;

    buf_offset -= 4;
    *(uint32_t *)(page_buf + buf_offset) = argv_vaddr;

    buf_offset -= 4;
    *(uint32_t *)(page_buf + buf_offset) = (uint32_t)argc;

    uint32_t initial_user_esp = (USER_STACK_TOP - PAGE_SIZE) + buf_offset;

    copy_to_phys(stack_phys1, 0, page_buf, PAGE_SIZE);

    *heap_start_out = (highest + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    *stack_start_out = initial_user_esp;

    return ehdr->e_entry;
}

process_t *uprocess_create(uint32_t entry, uint32_t *page_dir_phys, uint32_t heap_start, uint32_t initial_user_esp) {
    process_t *p = kmalloc(sizeof(process_t));

    p->pid = next_pid++;
    p->page_dir = page_dir_phys;
    p->entry = entry;
    p->state = PROCESS_RUNNING;
    p->parent = NULL;
    p->exit_code = 0;

    memset(&p->fds, 0, sizeof(fd_table_t));

    thread_t *t = kmalloc(sizeof(thread_t));

    t->stack_size = KSTACK_SIZE;
    t->stack_base = kalloc_pages(KSTACK_SIZE / PAGE_SIZE);
    t->state = THREAD_READY;
    t->process = p;
    t->next = NULL;

    uint32_t *sp = (uint32_t *)((uint32_t)t->stack_base + KSTACK_SIZE);

    *--sp = SEG_UDATA;
    *--sp = initial_user_esp;
    *--sp = 0x202;
    *--sp = SEG_UCODE;
    *--sp = entry;

    *--sp = 0;
    *--sp = 32;

    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;

    *--sp = SEG_UDATA;
    *--sp = SEG_UDATA;
    *--sp = SEG_UDATA;
    *--sp = SEG_UDATA;

    t->esp = (uint32_t)sp;

    p->thread = t;
    p->heap_start = heap_start;
    p->heap_end = heap_start;

    process_list_add(p);

    return p;
}