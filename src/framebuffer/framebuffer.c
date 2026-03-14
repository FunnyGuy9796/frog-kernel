#include "framebuffer.h"

mb2_tag_framebuffer_t *fb_tag = 0;
framebuffer_t fb;

static uint8_t fb_initialized = 0;

void fb_parse_tag(void *mb2_info) {
    mb2_tag_t *tag = (mb2_tag_t *)((uint8_t *)mb2_info + 8);

    while (tag->type != MB2_TAG_TYPE_END) {
        if (tag->type == MB2_TAG_TYPE_FRAMEBUFFER) {
            fb_tag = (mb2_tag_framebuffer_t *)tag;

            return;
        }

        tag = (mb2_tag_t *)(((uint32_t)tag + tag->size + 7) & ~7);
    }

    panic(NULL, "fb_parse_tag() -> no framebuffer tag found in multiboot2 info");
}

void fb_init() {
    uint32_t width = fb_tag->framebuffer_width;
    uint32_t height = fb_tag->framebuffer_height;
    uint32_t pitch = fb_tag->framebuffer_pitch;
    uint32_t bpp = fb_tag->framebuffer_bpp;

    uint32_t fb_phys = (uint32_t)fb_tag->framebuffer_addr;
    uint32_t fb_size = height * pitch;
    uint32_t pages = (fb_size + 0xfff) / 0x1000;

    for (uint32_t i = 0; i < pages; i++) {
        uint32_t phys = fb_phys + i * 0x1000;
        uint32_t virt = FRAMEBUFFER_OFFSET + i * 0x1000;

        map_page(phys, virt, PAGE_PRESENT | PAGE_RW | PAGE_PCD | PAGE_PWT);
    }

    asm volatile(
        "mov %%cr3, %%eax;"
        "mov %%eax, %%cr3;"
        ::: "eax", "memory"
    );

    fb.phys_addr = FRAMEBUFFER_OFFSET;
    fb.width = width;
    fb.height = height;
    fb.pitch = pitch;
    fb.bpp = bpp;
    fb.size = fb_size;

    uint32_t num_pages = (fb.size + PAGE_SIZE - 1) / PAGE_SIZE;

    fb.addr = (uint32_t)kalloc_pages(num_pages);

    if (!fb.addr)
        panic(NULL, "fb_init() -> failed to allocate back buffer");
    
    fb_initialized = 1;
}

void fb_swap() {
    if (!fb_initialized)
        return;
    
    uint32_t count = fb.size / 4;
    uint32_t *src = (uint32_t *)fb.addr;
    uint32_t *dst = (uint32_t *)fb.phys_addr;

    __asm__ volatile ("rep movsl" : "+S"(src), "+D"(dst), "+c"(count) :: "memory");
}

void fb_swap_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    for (uint32_t row = y; row < y + h && row < fb.height; row++) {
        uint32_t *src = (uint32_t *)(fb.addr + row * fb.pitch + x * 4);
        uint32_t *dst = (uint32_t *)(fb.phys_addr + row * fb.pitch + x * 4);
        uint32_t count = w;

        __asm__ volatile ("rep movsl" : "+S"(src), "+D"(dst), "+c"(count) :: "memory");
    }
}

void fb_clear(uint32_t color) {
    uint32_t *dest = (uint32_t *)(uint32_t)fb.addr;
    uint32_t count = fb.height * (fb.pitch / 4);

    __asm__ volatile ("rep stosl" : "+D"(dest), "+c"(count) : "a"(color) : "memory");
}

void fb_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg) {
    const uint8_t *glyph = font[(uint8_t)c];

    for (int row = 0; row < 16; row++) {
        uint8_t bits = glyph[row];

        for (int col = 0; col < 8; col++) {
            uint32_t color = (bits & (0x80 >> col)) ? fg : bg;
            
            fb_put_pixel(x + col, y + row, color);
        }
    }
}

void fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t pos_y = y; pos_y < y + h; pos_y++) {
        for (uint32_t pos_x = x; pos_x < x + w; pos_x++)
            fb_put_pixel(pos_x, pos_y, color);
    }
}