#ifndef TTY_H
#define TTY_H

#include "../../devfs/devfs.h"

#define TTY_GET_INFO 0x01
#define TTY_RING_SIZE 256

typedef struct {
    uintptr_t cols;
    uintptr_t rows;
    uint32_t bg_color;
    uint32_t fg_color;
    uint32_t cursor_x;
    uint32_t cursor_y;
} tty_info_t;

typedef struct {
    char ring[TTY_RING_SIZE];
    size_t ring_read;
    size_t ring_write;
    size_t ring_count;
    size_t line_len;
    tty_info_t info;
} tty_t;

void tty_register(uintptr_t cols, uintptr_t rows, uint32_t bg_color, uint32_t fg_color);
void tty_input_push(char c);

#endif