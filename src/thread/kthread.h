#ifndef KTHREAD_H
#define KTHREAD_H

#include <stdint.h>
#include "../misc/printf.h"
#include "../misc/panic.h"
#include "../memory/kheap.h"
#include "../gdt/gdt.h"

#define KSTACK_SIZE 0x2000

typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_DEAD
} thread_state_t;

typedef struct process process_t;

typedef struct thread {
    uint32_t esp;
    uint32_t *stack_base;
    uint32_t stack_size;
    thread_state_t state;
    process_t *process;
    struct thread *next;
} thread_t;

extern thread_t *idle_thread;
extern thread_t *curr_thread;
extern volatile uint8_t schedule_pending;

thread_t *kthread_create(void (*entry)(void), uint32_t stack_size);
void scheduler_add(thread_t *t);
thread_t *scheduler_next();
uint32_t scheduler_switch(uint32_t old_esp);

#endif