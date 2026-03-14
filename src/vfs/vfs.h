#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>
#include "fdtable.h"
#include "stat.h"
#include "../misc/util.h"
#include "../memory/kheap.h"

typedef ptrdiff_t ssize_t;

#define MAX_MOUNTS 64
#define MAX_PATH 256

typedef struct vnode vnode_t;
typedef struct fs_mount fs_mount_t;
typedef struct file file_t;

typedef struct {
    int (*open)(vnode_t *node, int flags);
    int (*close)(vnode_t *node);
    ssize_t (*read)(vnode_t *node, void *buf, size_t len, size_t offset);
    ssize_t (*write)(vnode_t *node, const void *buf, size_t len, size_t offset);
    int (*readdir)(vnode_t *node, size_t index, char *name_out, size_t name_max);
    vnode_t *(*finddir)(vnode_t *node, const char *name);
    int (*ioctl)(vnode_t *node, int request, void *arg);
    int (*mmap)(vnode_t *node, void *proc, uintptr_t virt, size_t len, int flags);
    int (*stat)(vnode_t *node, struct stat *st);
} vnode_ops_t;

#define VFS_FILE      0x01
#define VFS_DIRECTORY 0x02
#define VFS_CHARDEV   0x04
#define VFS_BLOCKDEV  0x08
#define VFS_PIPE      0x10
#define VFS_SYMLINK   0x20
#define VFS_MOUNTPT   0x40

typedef struct vnode {
    uint32_t inode;
    uint32_t flags;
    uint32_t uid, gid;
    uint32_t size;
    uint32_t permissions;
    vnode_ops_t *ops;
    fs_mount_t *mount;
    vnode_t *mounted_here;
    void *private_data;
} vnode_t;

typedef struct file {
    vnode_t *vnode;
    size_t offset;
    int flags;
    int ref_count;
} file_t;

typedef struct fs_driver {
    const char *name;
    int (*mount)(fs_mount_t *mnt, const char *device);
    int (*unmount)(fs_mount_t *mnt);
    vnode_t *(*get_root)(fs_mount_t *mnt);
} fs_driver_t;

typedef struct fs_mount {
    char path[MAX_PATH];
    fs_driver_t *driver;
    vnode_t *root;
    void *private;
} fs_mount_t;

struct process;
extern struct process *curr_process;

extern fs_mount_t mount_table[MAX_MOUNTS];
extern int mount_count;
extern fd_table_t kernel_fd_table;

int vfs_mount(const char *path, fs_driver_t *driver, const char *device);
int vfs_unmount(const char *path);
int vfs_open(const char *path, int flags);
int vfs_close(int fd);
int vfs_stat(int fd, struct stat *st);
ssize_t vfs_read(int fd, void *buf, size_t len);
ssize_t vfs_write(int fd, const void *buf, size_t len);
int vfs_readdir(int fd, size_t index, char *name_out, size_t name_max);
int vfs_ioctl(int fd, int request, void *arg);
vnode_t *vfs_resolve(const char *path);

#endif