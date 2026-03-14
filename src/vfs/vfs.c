#include "vfs.h"
#include "../userspace/uprocess.h"

fs_mount_t mount_table[MAX_MOUNTS] = { 0 };
int mount_count = 0;
fd_table_t kernel_fd_table = { 0 };

static fd_table_t *active_fd_table() {
    if (curr_process)
        return &curr_process->fds;

    return &kernel_fd_table;
}

static int alloc_fd(file_t *file) {
    fd_table_t *fds = active_fd_table();

    for (int i = 0; i < MAX_FDS; i++) {
        if (!fds->entries[i]) {
            fds->entries[i] = file;

            return i;
        }
    }

    return -1;
}

static file_t *get_file(int fd) {
    if (fd < 0 || fd >= MAX_FDS)
        return NULL;
    
    return active_fd_table()->entries[fd];
}

static fs_mount_t *find_mount(const char *path) {
    fs_mount_t *best = NULL;
    size_t best_len = 0;

    for (int i = 0; i < mount_count; i++) {
        size_t mlen = strlen(mount_table[i].path);

        if (strncmp(mount_table[i].path, path, mlen) == 0) {
            if (mlen > best_len) {
                best = &mount_table[i];
                best_len = mlen;
            }
        }
    }

    return best;
}

int vfs_mount(const char *path, fs_driver_t *driver, const char *device) {
    if (mount_count >= MAX_MOUNTS)
        return -1;
    
    fs_mount_t *mnt = &mount_table[mount_count];

    strncpy(mnt->path, path, MAX_PATH - 1);

    mnt->driver = driver;

    if (driver->mount(mnt, device) < 0)
        return -1;
    
    mnt->root = driver->get_root(mnt);

    if (!mnt->root)
        return -1;
    
    mount_count++;

    return 0;
}

int vfs_unmount(const char *path) {
    int idx = -1;

    for (int i = 0; i < mount_count; i++) {
        if (strncmp(mount_table[i].path, path, MAX_PATH) == 0) {
            idx = i;

            break;
        }
    }

    if (idx < 0)
        return -1;
    
    fs_mount_t *mnt = &mount_table[idx];

    for (int i = 0; i < MAX_FDS; i++) {
        file_t *file = kernel_fd_table.entries[i];

        if (file && file->vnode && file->vnode->mount == mnt)
            return -1;
    }

    if (idx > 0) {
        vnode_t *parent = vfs_resolve(path);

        if (parent && parent->flags & VFS_MOUNTPT)
            parent->mounted_here = NULL;
    }

    if (mnt->driver && mnt->driver->unmount)
        mnt->driver->unmount(mnt);
    
    for (int i = idx; i < mount_count - 1; i++)
        mount_table[i] = mount_table[i + 1];
    
    memset(&mount_table[mount_count - 1], 0, sizeof(fs_mount_t));

    mount_count--;

    return 0;
}

int vfs_open(const char *path, int flags) {
    vnode_t *node = vfs_resolve(path);

    if (!node)
        return -1;
    
    if (node->ops && node->ops->open) {
        if (node->ops->open(node, flags) < 0)
            return -1;
    }

    file_t *file = kmalloc(sizeof(file_t));

    if (!file)
        return -1;
    
    file->vnode = node;
    file->offset = 0;
    file->flags = flags;
    file->ref_count = 1;

    int fd = alloc_fd(file);

    if (fd < 0) {
        kfree(file);

        return -1;
    }

    return fd;
}

int vfs_close(int fd) {
    file_t *file = get_file(fd);

    if (!file)
        return -1;
    
    file->ref_count--;

    if (file->ref_count == 0) {
        if (file->vnode->ops && file->vnode->ops->close)
            file->vnode->ops->close(file->vnode);
        
        kfree(file);
    }

    kernel_fd_table.entries[fd] = NULL;

    return 0;
}

int vfs_stat(int fd, struct stat *st) {
    file_t *file = get_file(fd);

    if (!file)
        return -1;
    
    if (!file->vnode->ops || !file->vnode->ops->stat)
        return -1;
    
    return file->vnode->ops->stat(file->vnode, st);
}

ssize_t vfs_read(int fd, void *buf, size_t len) {
    file_t *file = get_file(fd);

    if (!file)
        return -1;
    
    if (!file->vnode->ops || !file->vnode->ops->read)
        return -1;
    
    ssize_t n = file->vnode->ops->read(file->vnode, buf, len, file->offset);

    if (n > 0)
        file->offset += n;
    
    return n;
}

ssize_t vfs_write(int fd, const void *buf, size_t len) {
    file_t *file = get_file(fd);

    if (!file)
        return -1;
    
    if (!file->vnode->ops || !file->vnode->ops->write)
        return -1;
    
    ssize_t n = file->vnode->ops->write(file->vnode, buf, len, file->offset);

    if (n > 0)
        file->offset += n;
    
    return n;
}

int vfs_readdir(int fd, size_t index, char *name_out, size_t name_max) {
    file_t *file = get_file(fd);

    if (!file)
        return -1;
    
    if (!(file->vnode->flags & VFS_DIRECTORY))
        return -1;
    
    if (!file->vnode->ops || !file->vnode->ops->readdir)
        return -1;
    
    return file->vnode->ops->readdir(file->vnode, index, name_out, name_max);
}

int vfs_ioctl(int fd, int request, void *arg) {
    file_t *file = get_file(fd);

    if (!file)
        return -1;
    
    if (!file->vnode->ops || !file->vnode->ops->ioctl)
        return -1;
    
    return file->vnode->ops->ioctl(file->vnode, request, arg);
}

vnode_t *vfs_resolve(const char *path) {
    fs_mount_t *mnt = find_mount(path);

    if (!mnt || !mnt->root)
        return NULL;
    
    const char *rel = path + strlen(mnt->path);
    vnode_t *node = mnt->root;
    char component[MAX_PATH];

    while (*rel) {
        while (*rel == '/')
            rel++;
        
        if (!*rel)
            break;
        
        size_t i = 0;

        while (*rel && *rel != '/')
            component[i++] = *rel++;
        
        component[i] = '\0';
        
        if (!(node->flags & (VFS_DIRECTORY | VFS_MOUNTPT)))
            return NULL;
        
        if (node->mounted_here)
            node = node->mounted_here;
        
        if (!node->ops || !node->ops->finddir)
            return NULL;
        
        node = node->ops->finddir(node, component);

        if (!node)
            return NULL;
    }

    if (node->flags & VFS_MOUNTPT && node->mounted_here)
        node = node->mounted_here;
    
    return node;
}