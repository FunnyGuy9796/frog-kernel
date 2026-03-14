#include "null.h"

static ssize_t null_read(vnode_t *node, void *buf, size_t len, size_t offset) {
    (void)node;
    (void)buf;
    (void)len;
    (void)offset;

    return 0;
}

static ssize_t null_write(vnode_t *node, const void *buf, size_t len, size_t offset) {
    (void)node;
    (void)buf;
    (void)offset;

    return (ssize_t)len;
}

static vnode_ops_t null_ops = {
    .read = null_read,
    .write = null_write
};

void null_init() {
    devfs_register("null", &null_ops, NULL, VFS_CHARDEV);
}