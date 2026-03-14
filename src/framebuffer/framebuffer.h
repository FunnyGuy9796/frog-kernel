#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "../multiboot.h"
#include "../misc/util.h"
#include "../misc/panic.h"
#include "../memory/paging.h"
#include "../memory/kheap.h"
#include "font.h"

typedef struct {
    uint32_t addr;
    uint32_t phys_addr;
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t pitch;
    uint32_t size;
} framebuffer_t;

extern mb2_tag_framebuffer_t *fb_tag;
extern framebuffer_t fb;

void fb_parse_tag(void *mb2_info);
void fb_init();
void fb_swap();
void fb_swap_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void fb_clear(uint32_t color);

static inline uint32_t fb_get_pixel(uint32_t x, uint32_t y) {
    uint32_t *pixel = (uint32_t *)((uint8_t *)(uint32_t)fb.addr + y * fb.pitch + x * (fb.bpp / 8));

    return *pixel;
}

static inline void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    uint32_t *pixel = (uint32_t *)((uint8_t *)(uint32_t)fb.addr + y * fb.pitch + x * (fb.bpp / 8));

    *pixel = color;
}

void fb_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg);
void fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);

#endif