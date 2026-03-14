#ifndef FB_H
#define FB_H

#include "../../devfs/devfs.h"
#include "../../framebuffer/framebuffer.h"

#define FB_GET_INFO 0x01

typedef struct {
    uintptr_t phys_addr;
    uintptr_t virt_addr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
} fb_info_t;

void fb_register(uintptr_t phys, uintptr_t virt, uint32_t w, uint32_t h, uint32_t pitch, uint32_t bpp);

#endif