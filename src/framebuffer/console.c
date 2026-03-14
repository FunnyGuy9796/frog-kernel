#include "console.h"

uint32_t console_cols;
uint32_t console_rows;
uint32_t console_bg;
uint32_t console_fg;
int cursor_x;
int cursor_y;

static uint8_t cursor_visible = 0;

void console_init() {
    console_cols = fb.width / 8;
    console_rows = fb.height / 16;
    console_bg = 0xff233c67;
    console_fg = 0xffededed;
    cursor_x = 0;
    cursor_y = 0;

    fb_clear(console_bg);
}

void console_scroll() {
    uint32_t row_bytes = fb.pitch * 16;
    uint8_t *fb8 = (uint8_t *)fb.addr;
    uint32_t total_bytes = fb.pitch * fb.height;

    memmove(fb8, fb8 + row_bytes, total_bytes - row_bytes);
    fb_fill_rect(0, fb.height - 16, fb.width, 16, console_bg);
}

void console_draw_cursor() {
    fb_fill_rect(cursor_x * 8, cursor_y * 16, 8, 16, console_fg);
}

void console_clear_cursor() {
    fb_fill_rect(cursor_x * 8, cursor_y * 16, 8, 16, console_bg);
}

void console_update_cursor() {
    if (cursor_visible)
        console_draw_cursor();
    else
        console_clear_cursor();
}

void console_toggle_cursor() {
    cursor_visible = !cursor_visible;

    console_update_cursor();
}

void console_putchar(char c) {
    console_clear_cursor();

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r')
        cursor_x = 0;
    else if (c == '\b') {
        if (cursor_x > 0)
            cursor_x--;
        else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = console_cols - 1;
        }
        
        fb_draw_char(cursor_x * 8, cursor_y * 16, ' ', console_fg, console_bg);
    } else {
        if (cursor_x >= console_cols - 1) {
            cursor_x = 0;
            cursor_y++;
        }
        
        fb_draw_char(cursor_x * 8, cursor_y * 16, c, console_fg, console_bg);

        cursor_x++;
    }

    if (cursor_y >= console_rows) {
        console_scroll();

        cursor_y = console_rows - 1;
    }

    console_draw_cursor();

    cursor_visible = 1;
}