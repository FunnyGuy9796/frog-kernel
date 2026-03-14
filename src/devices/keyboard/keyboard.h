#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "../../misc/util.h"

#define KEYBOARD_BUF_SIZE 64
#define KEY_RELEASED 0x80

#define L_SHIFT (1 << 0)
#define R_SHIFT (1 << 1)
#define L_CTRL (1 << 2)
#define R_CTRL (1 << 3)
#define L_ALT (1 << 4)
#define R_ALT (1 << 5)

typedef struct registers registers_t;

typedef struct {
    char key;
    int pressed;
    uint32_t mods;
} keyboard_state_t;

typedef struct {
    uint8_t buf[KEYBOARD_BUF_SIZE];
    uint32_t head;
    uint32_t tail;
} keyboard_buf_t;

extern volatile uint8_t keyboard_dirty;
extern keyboard_state_t keyboard;
extern keyboard_buf_t keyboard_buf;

void keyboard_init();
void keyboard_callback(registers_t *regs);

#endif