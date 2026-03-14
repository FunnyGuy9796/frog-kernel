#include "devfs.h"

typedef struct {
    char name[64];
    vnode_ops_t *ops;
    void *private_data;
    uint32_t flags;
    int active;
} devfs_entry_t;

static devfs_entry_t registry[DEVFS_MAX_DEVICES] = { 0 };
static int registry_count = 0;

int devfs_register(const char *name, vnode_ops_t *ops, void *private_data, uint32_t flags) {
    if (registry_count >= DEVFS_MAX_DEVICES)
        return -1;
    
    devfs_entry_t *entry = &registry[registry_count++];

    strncpy(entry->name, name, 63);

    entry->name[63] = '\0';
    entry->ops = ops;
    entry->private_data = private_data;
    entry->flags = flags;
    entry->active = 1;

    return 0;
}

static vnode_t *devfs_vnode_from_entry(devfs_entry_t *entry) {
    vnode_t *node = kmalloc(sizeof(vnode_t));

    if (!node)
        return NULL;
    
    memset(node, 0, sizeof(vnode_t));

    node->ops = entry->ops;
    node->flags = entry->flags;
    node->private_data = entry->private_data;

    return node;
}

static vnode_t *devfs_root_finddir(vnode_t *node, const char *name) {
    (void)node;

    for (int i = 0; i < registry_count; i++) {
        if (registry[i].active && strcmp(registry[i].name, name) == 0)
            return devfs_vnode_from_entry(&registry[i]);
    }

    return NULL;
}

static int devfs_root_readdir(vnode_t *node, size_t index, char *name_out, size_t name_max) {
    (void)node;

    size_t found = 0;

    for (int i = 0; i < registry_count; i++) {
        if (!registry[i].active)
            continue;
        
        if (found == index) {
            strncpy(name_out, registry[i].name, name_max - 1);

            name_out[name_max - 1] = '\0';

            return 0;
        }

        found++;
    }

    return -1;
}

static vnode_ops_t devfs_root_ops = {
    .finddir = devfs_root_finddir,
    .readdir = devfs_root_readdir
};

static vnode_t *devfs_root = NULL;

static int devfs_mount(fs_mount_t *mnt, const char *device) {
    (void)mnt;
    (void)device;

    devfs_root = kmalloc(sizeof(vnode_t));

    if (!devfs_root)
        return -1;
    
    memset(devfs_root, 0, sizeof(vnode_t));

    devfs_root->flags = VFS_DIRECTORY;
    devfs_root->ops = &devfs_root_ops;

    return 0;
}

static int devfs_unmount(fs_mount_t *mnt) {
    (void)mnt;

    kfree(devfs_root);

    devfs_root = NULL;

    return 0;
}

static vnode_t *devfs_get_root(fs_mount_t *mnt) {
    (void)mnt;

    return devfs_root;
}

static fs_driver_t devfs_driver = {
    .name = "devfs",
    .mount = devfs_mount,
    .unmount = devfs_unmount,
    .get_root = devfs_get_root
};

fs_driver_t *devfs_get_driver() {
    return &devfs_driver;
}