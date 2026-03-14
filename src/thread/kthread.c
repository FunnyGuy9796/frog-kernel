#include "kthread.h"
#include "../userspace/uprocess.h"

thread_t *idle_thread = NULL;
thread_t *curr_thread = NULL;
volatile uint8_t schedule_pending = 0;

static thread_t *thread_list_head = NULL;

thread_t *kthread_create(void (*entry)(void), uint32_t stack_size) {
    thread_t *t = kmalloc(sizeof(thread_t));

    t->stack_base = kalloc_pages((stack_size + PAGE_SIZE - 1) / PAGE_SIZE);
    t->stack_size = stack_size;

    uint32_t *sp = (uint32_t *)((uint32_t)t->stack_base + stack_size);

    *--sp = 0x00200202;
    *--sp = SEG_KCODE;
    *--sp = (uint32_t)entry;

    *--sp = 32;
    *--sp = 0;

    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;
    *--sp = 0;

    *--sp = SEG_KDATA;
    *--sp = SEG_KDATA;
    *--sp = SEG_KDATA;
    *--sp = SEG_KDATA;

    t->esp = (uint32_t)sp;
    t->process = NULL;
    t->next = NULL;
    t->state = THREAD_READY;

    return t;
}

void scheduler_add(thread_t *t) {
    if (!thread_list_head) {
        thread_list_head = t;
        t->next = t;
    } else {
        thread_t *tail = thread_list_head;

        while (tail->next != thread_list_head)
            tail = tail->next;

        tail->next = t;
        t->next = thread_list_head;
    }
}

thread_t *scheduler_next() {
    if (curr_thread == NULL)
        return idle_thread;
    
    thread_t *t = curr_thread->next;
    thread_t *start = t;

    while (t->state != THREAD_READY && t->state != THREAD_RUNNING) {
        t = t->next;

        if (t == start)
            return idle_thread;
    }

    return t;
}

uint32_t scheduler_switch(uint32_t old_esp) {
    curr_thread->esp = old_esp;

    thread_t *old  = curr_thread;
    thread_t *next = scheduler_next();

    if (next == old)
        return old_esp;

    old->state  = THREAD_READY;
    next->state = THREAD_RUNNING;
    curr_thread = next;

    curr_process = next->process;

    tss_set_kernel_stack((uint32_t)next->stack_base + next->stack_size);

    uint32_t *next_pd = next->process ? next->process->page_dir : NULL;
    uint32_t *old_pd = old->process ? old->process->page_dir : NULL;

    if (next_pd)
        __asm__ volatile ("mov %0, %%cr3" :: "r"(next_pd) : "memory");
    else if (old_pd)
        __asm__ volatile ("mov %0, %%cr3" :: "r"(kernel_pd_phys) : "memory");

    return next->esp;
}