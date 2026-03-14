#ifndef CONSOLE_H
#define CONSOLE_H

#include "../misc/mem.h"
#include "../devices/mouse/mouse.h"
#include "../devices/keyboard/keyboard.h"

extern uint32_t console_cols;
extern uint32_t console_rows;
extern uint32_t console_bg;
extern uint32_t console_fg;
extern int cursor_x;
extern int cursor_y;

void console_init();
void console_scroll();
void console_draw_cursor();
void console_clear_cursor();
void console_update_cursor();
void console_toggle_cursor();
void console_putchar(char c);

#endif