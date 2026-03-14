#include "tty.h"
#include "../../framebuffer/console.h"
#include "../../thread/kthread.h"

static tty_t tty = { 0 };

void tty_input_push(char c) {
    if (c == '\b') {
        if (tty.line_len == 0)
            return;
        
        if (tty.ring_count > 0) {
            tty.ring_write = (tty.ring_write + TTY_RING_SIZE - 1) % TTY_RING_SIZE;
            tty.ring_count--;
            tty.line_len--;
        }

        console_putchar('\b');

        return;
    }

    if (tty.ring_count >= TTY_RING_SIZE)
        return;
    
    tty.ring[tty.ring_write] = c;
    tty.ring_write = (tty.ring_write + 1) % TTY_RING_SIZE;
    tty.ring_count++;

    if (c == '\n')
        tty.line_len = 0;
    else
        tty.line_len++;

    console_putchar(c);

    if (curr_thread->state == THREAD_BLOCKED) {
        curr_thread->state = THREAD_READY;
        schedule_pending = 1;
    }
}

static ssize_t tty_read(vnode_t *node, void *buf, size_t len, size_t offset) {
    (void)offset;

    tty_t *t = node->private_data;
    char *out = buf;
    size_t n = 0;

    while (1) {
        while (t->ring_count == 0) {
            curr_thread->state = THREAD_BLOCKED;
            schedule_pending = 1;

            __asm__ volatile ("sti\n\thlt\n\tcli");

            curr_thread->state = THREAD_RUNNING;
        }

        int has_newline = 0;

        for (size_t i = 0; i < t->ring_count; i++) {
            size_t idx = (t->ring_read + i) % TTY_RING_SIZE;

            if (t->ring[idx] == '\n') {
                has_newline = 1;

                break;
            }
        }

        if (has_newline)
            break;
        
        curr_thread->state = THREAD_BLOCKED;
        schedule_pending = 1;

        __asm__ volatile ("sti\n\thlt\n\tcli");

        curr_thread->state = THREAD_RUNNING;
    }
    
    while (n < len && t->ring_count > 0) {
        char c = t->ring[t->ring_read];

        t->ring_read = (t->ring_read + 1) % TTY_RING_SIZE;
        t->ring_count--;
        out[n++] = c;

        if (c == '\n')
            break;
    }

    return (ssize_t)n;
}

static ssize_t tty_write(vnode_t *node, const void *buf, size_t len, size_t offset) {
    (void)offset;
    
    tty_t *t = node->private_data;
    const char *str = buf;

    for (size_t i = 0; i < len; i++)
        console_putchar(str[i]);
    
    t->info.cursor_x = cursor_x;
    t->info.cursor_y = cursor_y;

    return (ssize_t)len;
}

static int tty_ioctl(vnode_t *node, int request, void *arg) {
    tty_t *t = node->private_data;

    switch (request) {
        case TTY_GET_INFO: {
            t->info.cursor_x = cursor_x;
            t->info.cursor_y = cursor_y;

            memcpy(arg, &t->info, sizeof(tty_info_t));

            return 0;
        }

        default:
            return -1;   
    }
}

static vnode_ops_t tty_ops = {
    .read = tty_read,
    .write = tty_write,
    .ioctl = tty_ioctl
};

void tty_register(uintptr_t cols, uintptr_t rows, uint32_t bg_color, uint32_t fg_color) {
    tty.info.cols = cols;
    tty.info.rows = rows;
    tty.info.bg_color = bg_color;
    tty.info.fg_color = fg_color;
    tty.info.cursor_x = 0;
    tty.info.cursor_y = 0;

    devfs_register("tty0", &tty_ops, &tty, VFS_CHARDEV);
}