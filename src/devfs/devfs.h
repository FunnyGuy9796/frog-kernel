#ifndef DEVFS_H
#define DEVFS_H

#include "../vfs/vfs.h"

#define DEVFS_MAX_DEVICES 64

int devfs_register(const char *name, vnode_ops_t *ops, void *private_data, uint32_t flags);
fs_driver_t *devfs_get_driver();

#endif