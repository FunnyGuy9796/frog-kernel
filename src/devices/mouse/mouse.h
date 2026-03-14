#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include "../../misc/util.h"
#include "../../framebuffer/framebuffer.h"

#define CURSOR_W 16
#define CURSOR_H 24

typedef struct registers registers_t;

typedef struct {
    int32_t x, y;
    bool left;
    bool right;
    bool middle;
    bool left_clicked;
    bool right_clicked;
    bool middle_clicked;
} mouse_state_t;

extern mouse_state_t mouse;

void mouse_init();
void mouse_callback(registers_t *regs);

#endif