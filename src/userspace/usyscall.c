#include "usyscall.h"

static int val_ptr(const void *ptr, size_t len) {
    uint32_t addr = (uint32_t)ptr;

    if (addr >= KERNEL_VIRT_BASE)
        return 0;
    
    if (addr + len >= KERNEL_VIRT_BASE)
        return 0;
    
    if (!addr)
        return 0;
    
    return 1;
}

static void sys_open(registers_t *regs) {
    const char *path = (const char *)regs->ebx;
    int flags = (int)regs->ecx;

    if (!val_ptr(path, 1)) {
        regs->eax = -1;

        return;
    }

    if (!val_ptr(path, strnlen(path, MAX_PATH))) {
        regs->eax = -1;

        return;
    }

    regs->eax = (uint32_t)vfs_open(path, flags);
}

static void sys_close(registers_t *regs) {
    regs->eax = (uint32_t)vfs_close((int)regs->ebx);
}

static void sys_read(registers_t *regs) {
    int fd = (int)regs->ebx;
    void *buf = (void *)regs->ecx;
    size_t len = (size_t)regs->edx;

    if (!val_ptr(buf, len)) {
        regs->eax = -1;

        return;
    }

    regs->eax = (uint32_t)vfs_read(fd, buf, len);
}

static void sys_write(registers_t *regs) {
    int fd = (int)regs->ebx;
    const void *buf = (const void *)regs->ecx;
    size_t len = (size_t)regs->edx;

    if (!val_ptr(buf, len)) {
        regs->eax = -1;

        return;
    }

    ssize_t n = vfs_write(fd, buf, len);

    regs->eax = (uint32_t)n;
}

static void sys_ioctl(registers_t *regs) {
    int fd = (int)regs->ebx;
    int request = (int)regs->ecx;
    void *arg = (void *)regs->edx;

    if (arg && !val_ptr(arg, 1)) {
        regs->eax = -1;

        return;
    }

    regs->eax = (uint32_t)vfs_ioctl(fd, request, arg);
}

static void sys_readdir(registers_t *regs) {
    int fd = (int)regs->ebx;
    size_t index = (size_t)regs->ecx;
    char *name_out = (char *)regs->edx;
    size_t name_max = (size_t)regs->ebp;

    if (!val_ptr(name_out, name_max)) {
        regs->eax = -1;
        
        return;
    }

    regs->eax = (uint32_t)vfs_readdir(fd, index, name_out, name_max);
}

static void sys_fstat(registers_t *regs) {
    int fd = (int)regs->ebx;
    struct stat *st = (struct stat *)regs->ecx;

    if (!val_ptr(st, sizeof(struct stat))) {
        regs->eax = -1;
        
        return;
    }

    regs->eax = (uint32_t)vfs_stat(fd, st);
}

static void sys_exit(registers_t *regs) {
    int code = (int)regs->ebx;

    curr_process->exit_code = code;
    curr_process->state = PROCESS_DEAD;
    curr_thread->state = THREAD_DEAD;

    if (curr_process->parent && curr_process->parent->state == PROCESS_WAITING) {
        curr_process->parent->state = PROCESS_RUNNING;
        curr_process->parent->thread->state = THREAD_READY;
    }

    schedule_pending = 1;

    __asm__ volatile ("sti");

    while (1)
        __asm__ volatile ("hlt");
}

static void sys_time(registers_t *regs) {
    rtc_time_t *user_time = (rtc_time_t *)regs->ebx;

    if (!val_ptr(user_time, sizeof(rtc_time_t))) {
        regs->eax = -1;

        return;
    }

    user_time->seconds = curr_time.seconds;
    user_time->minutes = curr_time.minutes;
    user_time->hours = curr_time.hours;
    user_time->day = curr_time.day;
    user_time->month = curr_time.month;
    user_time->year = curr_time.year;

    regs->eax = 0;
}

static void sys_spawn(registers_t *regs) {
    const char *path = (const char *)regs->ebx;
    char **user_argv = (char **)regs->ecx;
    int argc = (int)regs->edx;

    if (!val_ptr(path, 1) || !val_ptr(path, strnlen(path, MAX_PATH))) {
        regs->eax = (uint32_t)-1;

        return;
    }

    if (argc < 0 || argc > MAX_ARGS) {
        regs->eax = (uint32_t)-1;

        return;
    }

    if (!val_ptr(user_argv, argc * sizeof(char *))) {
        regs->eax = (uint32_t)-1;

        return;
    }

    char *kernel_argv[MAX_ARGS];
    char kernel_strs[MAX_ARGS][MAX_PATH];

    for (int i = 0; i < argc; i++) {
        char *user_str = user_argv[i];

        if (!val_ptr(user_str, 1)) {
            regs->eax = (uint32_t)-1;

            return;
        }

        size_t len = strnlen(user_str, MAX_PATH);

        if (!val_ptr(user_str, len)) {
            regs->eax = (uint32_t)-1;

            return;
        }

        memcpy(kernel_strs[i], user_str, len + 1);

        kernel_argv[i] = kernel_strs[i];
    }

    int fd = vfs_open(path, 0);

    if (fd < 0) {
        regs->eax = (uint32_t)-1;

        return;
    }

    struct stat st;

    if (vfs_stat(fd, &st) < 0) {
        vfs_close(fd);

        regs->eax = (uint32_t)-1;

        return;
    }

    uint8_t *buf = kmalloc(st.st_size);

    if (!buf) {
        vfs_close(fd);

        regs->eax = (uint32_t)-1;

        return;
    }

    if (vfs_read(fd, buf, st.st_size) < 0) {
        kfree(buf);
        vfs_close(fd);

        regs->eax = (uint32_t)-1;

        return;
    }

    vfs_close(fd);

    uint32_t heap_start;
    uint32_t init_esp;
    uint32_t *pd_phys = uprocess_create_pagedir();
    uint32_t entry = uprocess_load(pd_phys, buf, st.st_size, &heap_start, &init_esp, argc, kernel_argv);

    kfree(buf);

    process_t *child = uprocess_create(entry, pd_phys, heap_start, init_esp);

    child->parent = curr_process;

    scheduler_add(child->thread);

    regs->eax = (uint32_t)child->pid;
}

static void sys_wait(registers_t *regs) {
    uint32_t child_pid = (uint32_t)regs->ebx;
    int *exit_code = (int *)regs->ecx;

    if (exit_code && !val_ptr(exit_code, sizeof(int))) {
        regs->eax = (uint32_t)-1;

        return;
    }

    process_t *child = uprocess_find(child_pid);

    if (!child) {
        regs->eax = (uint32_t)-1;

        return;
    }

    if (child->parent != curr_process) {
        regs->eax = (uint32_t)-1;

        return;
    }

    if (child->state != PROCESS_DEAD) {
        curr_process->state = PROCESS_WAITING;
        curr_thread->state = THREAD_BLOCKED;
        schedule_pending = 1;

        __asm__ volatile ("sti");

        while (curr_process->state == PROCESS_WAITING)
            __asm__ volatile ("hlt");
    }

    if (exit_code)
        *exit_code = (int)child->exit_code;

    uprocess_remove(child_pid);
    kfree(child->thread->stack_base);
    kfree(child->thread);
    kfree(child);

    regs->eax = 0;
}

static void sys_getpid(registers_t *regs) {
    if (curr_process == NULL) {
        regs->eax = (uint32_t)-1;

        return;
    }

    regs->eax = curr_process->pid;
}

static void sys_getppid(registers_t *regs) {
    if (curr_process == NULL) {
        regs->eax = (uint32_t)-1;

        return;
    }

    if (curr_process->parent == NULL) {
        regs->eax = (uint32_t)-1;

        return;
    }

    regs->eax = curr_process->parent->pid;
}

void userspace_init() {
    syscall_register(SYS_EXIT, sys_exit);
    syscall_register(SYS_OPEN, sys_open);
    syscall_register(SYS_CLOSE, sys_close);
    syscall_register(SYS_READ, sys_read);
    syscall_register(SYS_WRITE, sys_write);
    syscall_register(SYS_IOCTL, sys_ioctl);
    syscall_register(SYS_READDIR, sys_readdir);
    syscall_register(SYS_FSTAT, sys_fstat);
    syscall_register(SYS_TIME, sys_time);
    syscall_register(SYS_SPAWN, sys_spawn);
    syscall_register(SYS_WAIT, sys_wait);
    syscall_register(SYS_GETPID, sys_getpid);
    syscall_register(SYS_GETPPID, sys_getppid);
}