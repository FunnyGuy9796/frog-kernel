#include "zero.h"

static ssize_t zero_read(vnode_t *node, void *buf, size_t len, size_t offset) {
    (void)node;
    (void)offset;

    memset(buf, 0, len);

    return (ssize_t)len;
}

static ssize_t zero_write(vnode_t *node, const void *buf, size_t len, size_t offset) {
    (void)node;
    (void)buf;
    (void)offset;

    return (ssize_t)len;
}

static vnode_ops_t zero_ops = {
    .read = zero_read,
    .write = zero_write
};

void zero_init() {
    devfs_register("zero", &zero_ops, NULL, VFS_CHARDEV);
}