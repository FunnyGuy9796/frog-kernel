#include "fb.h"

static ssize_t fb_read(vnode_t *node, void *buf, size_t len, size_t offset) {
    fb_info_t *fb = node->private_data;
    size_t fb_size = fb->pitch * fb->width;

    if (offset >= fb_size)
        return 0;
    
    size_t available = fb_size - offset;
    size_t to_read = len < available ? len : available;

    memcpy(buf, (uint8_t *)fb->virt_addr + offset, to_read);

    return (ssize_t)to_read;
}

static ssize_t fb_write(vnode_t *node, const void *buf, size_t len, size_t offset) {
    fb_info_t *fb = node->private_data;
    size_t fb_size = fb->pitch * fb->width;

    if (offset >= fb_size)
        return 0;
    
    size_t available = fb_size - offset;
    size_t to_write = len < available ? len : available;

    memcpy((uint8_t *)fb->virt_addr + offset, buf, to_write);

    return (ssize_t)to_write;
}

static int fb_ioctl(vnode_t *node, int request, void *arg) {
    fb_info_t *fb = node->private_data;

    switch (request) {
        case FB_GET_INFO: {
            memcpy(arg, fb, sizeof(fb_info_t));

            return 0;
        }

        default:
            return -1;   
    }
}

static vnode_ops_t fb_ops = {
    .read = fb_read,
    .write = fb_write,
    .ioctl = fb_ioctl
};

void fb_register(uintptr_t phys, uintptr_t virt, uint32_t w, uint32_t h, uint32_t pitch, uint32_t bpp) {
    fb_info_t *fb = kmalloc(sizeof(fb_info_t));

    fb->phys_addr = phys;
    fb->virt_addr = virt;
    fb->width = w;
    fb->height = h;
    fb->pitch = pitch;
    fb->bpp = bpp;

    devfs_register("fb0", &fb_ops, fb, VFS_CHARDEV);
}