#ifndef UPROCESS_H
#define UPROCESS_H

#include <stdint.h>
#include "../memory/paging.h"
#include "../memory/pmm.h"
#include "../misc/util.h"
#include "../ksym/elf.h"
#include "../thread/kthread.h"
#include "../vfs/fdtable.h"

#define USER_VIRT_BASE 0x00400000
#define USER_STACK_TOP 0xb0000000
#define USTACK_SIZE 0x4000

#define TEMP_PD_SLOT 1020
#define TEMP_PD_VIRT (RECURSIVE_BASE + TEMP_PD_SLOT * 0x1000)
#define TEMP_FRAME_SLOT 1021
#define TEMP_FRAME_VIRT (RECURSIVE_BASE + TEMP_FRAME_SLOT * 0x1000)
#define TEMP_PT_SLOT 1022
#define TEMP_PT_VIRT (RECURSIVE_BASE + TEMP_PT_SLOT * 0x1000)

#define MAX_ARGS 32

typedef enum {
    PROCESS_RUNNING,
    PROCESS_DEAD,
    PROCESS_WAITING
} process_state_t;

typedef struct process {
    uint32_t pid;
    uint32_t *page_dir;
    uint32_t entry;
    thread_t *thread;
    process_state_t state;
    process_t *parent;
    process_t *next;
    int exit_code;
    struct fd_table fds;
    uint32_t heap_start;
    uint32_t heap_end;
} process_t;

extern process_t *curr_process;

process_t *uprocess_find(uint32_t pid);
void uprocess_remove(uint32_t pid);
uint32_t *uprocess_create_pagedir();
uint32_t uprocess_load(uint32_t *pd_phys, const uint8_t *data, uint32_t size, uint32_t *heap_start_out, uint32_t *stack_start_out, int argc, char **argv);
process_t *uprocess_create(uint32_t entry, uint32_t *page_dir_phys, uint32_t heap_start, uint32_t initial_user_esp);

#endif