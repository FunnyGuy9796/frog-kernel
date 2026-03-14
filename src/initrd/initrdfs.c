#include "initrdfs.h"

typedef struct {
    char path[256];
} initrd_vnode_data_t;

static vnode_t *initrd_vnode_create(const char *path, uint8_t typeflag);

static ssize_t initrd_file_read(vnode_t *node, void *buf, size_t len, size_t offset) {
    initrd_vnode_data_t *data = node->private_data;
    initrd_file_t f = initrd_lookup(data->path);

    if (!f.data)
        return -1;
    
    if (offset >= f.size)
        return 0;
    
    size_t available = f.size - offset;
    size_t to_read = len < available ? len : available;

    memcpy(buf, f.data + offset, to_read);

    return (ssize_t)to_read;
}

static int initrd_file_stat(vnode_t *node, struct stat *st) {
    initrd_vnode_data_t *data = node->private_data;
    initrd_file_t f = initrd_lookup(data->path);

    if (!f.data)
        return -1;
    
    memset(st, 0, sizeof(struct stat));

    st->st_size = (off_t)f.size;
    st->st_mode = (node->flags & VFS_DIRECTORY) ? S_IFDIR : S_IFREG;

    return 0;
}

static vnode_ops_t initrd_file_ops = {
    .read = initrd_file_read,
    .stat = initrd_file_stat
};

static vnode_t *initrd_dir_finddir(vnode_t *node, const char *name) {
    initrd_vnode_data_t *data = node->private_data;
    char child_path[256];
    size_t dlen = strnlen(data->path, 255);

    if (dlen == 0 || (dlen == 1 && data->path[0] == '/'))
        ksnprintf(child_path, sizeof(child_path), "%s", name);
    else
        ksnprintf(child_path, sizeof(child_path), "%s/%s", data->path, name);
    
    uint8_t *ptr = (uint8_t *)(uintptr_t)initrd.addr;
    uint8_t *end = ptr + initrd.size;

    while (ptr + 512 <= end) {
        ustar_header_t *hdr = (ustar_header_t *)ptr;

        if (hdr->name[0] == '\0')
            break;
        
        if (memcmp(hdr->magic, "ustar", 5) != 0)
            break;
        
        char fullpath[256];

        if (hdr->prefix[0] != '\0') {
            size_t plen = strnlen(hdr->prefix, 155);
            size_t nlen = strnlen(hdr->name, 100);

            memcpy(fullpath, hdr->prefix, plen);

            fullpath[plen] = '/';

            memcpy(fullpath + plen + 1, hdr->name, nlen);

            fullpath[plen + 1 + nlen] = '\0';

            size_t total = plen + 1 + nlen;

            if (total > 1 && fullpath[total - 1] == '/')
                fullpath[total - 1] = '\0';
        } else {
            strncpy(fullpath, hdr->name, 255);

            fullpath[255] = '\0';

            size_t nlen = strnlen(fullpath, 255);

            if (nlen > 1 && fullpath[nlen - 1] == '/')
                fullpath[nlen - 1] = '\0';
        }

        if (strcmp(fullpath, child_path) == 0)
            return initrd_vnode_create(fullpath, hdr->typeflag);
        
        uint32_t filesize = oct2int(hdr->size, 12);

        ptr = (ptr + 512) + ((filesize + 511) & ~511);
    }

    return NULL;
}

static int initrd_dir_readdir(vnode_t *node, size_t index, char *name_out, size_t name_max) {
    initrd_vnode_data_t *data = node->private_data;
    size_t dir_len = strnlen(data->path, 255);
    uint8_t *ptr = (uint8_t *)(uintptr_t)initrd.addr;
    uint8_t *end = ptr + initrd.size;
    size_t found = 0;

    while (ptr + 512 <= end) {
        ustar_header_t *hdr = (ustar_header_t *)ptr;

        if (hdr->name[0] == '\0')
            break;
        
        if (memcmp(hdr->magic, "ustar", 5) != 0)
            break;
        
        char fullpath[256];

        if (hdr->prefix[0] != '\0') {
            size_t plen = strnlen(hdr->prefix, 155);
            size_t nlen = strnlen(hdr->name, 100);

            memcpy(fullpath, hdr->prefix, plen);

            fullpath[plen] = '/';

            memcpy(fullpath + plen + 1, hdr->name, nlen);

            fullpath[plen + 1 + nlen] = '\0';

            size_t total = plen + 1 + nlen;

            if (total > 1 && fullpath[total - 1] == '/')
                fullpath[total - 1] = '\0';
        } else {
            strncpy(fullpath, hdr->name, 255);

            fullpath[255] = '\0';

            size_t nlen = strnlen(fullpath, 255);

            if (nlen > 1 && fullpath[nlen - 1] == '/')
                fullpath[nlen - 1] = '\0';
        }

        uint32_t filesize = oct2int(hdr->size, 12);
        const char *rest = NULL;

        if (dir_len == 0)
            rest = fullpath;
        else if (strncmp(fullpath, data->path, dir_len) == 0 && fullpath[dir_len] == '/')
            rest = fullpath + dir_len + 1;
        
        if (rest && strnlen(rest, 256) > 0 && strchr(rest, '/') == NULL) {
            if (found == index) {
                strncpy(name_out, rest, name_max - 1);
                
                name_out[name_max - 1] = '\0';

                return 0;
            }

            found++;
        }

        ptr = (ptr + 512) + ((filesize + 511) & ~511);
    }

    return -1;
}

static vnode_ops_t initrd_dir_ops = {
    .finddir = initrd_dir_finddir,
    .readdir = initrd_dir_readdir
};

static vnode_t *initrd_vnode_create(const char *path, uint8_t typeflag) {
    vnode_t *node = kmalloc(sizeof(vnode_t));

    if (!node)
        return NULL;
    
    memset(node, 0, sizeof(vnode_t));

    initrd_vnode_data_t *priv = kmalloc(sizeof(initrd_vnode_data_t));

    if (!priv) {
        kfree(node);

        return NULL;
    }

    strncpy(priv->path, path, 255);

    priv->path[255] = '\0';
    node->private_data = priv;

    if (typeflag == '5') {
        node->flags = VFS_DIRECTORY;
        node->ops = &initrd_dir_ops;
    } else {
        initrd_file_t f = initrd_lookup(path);

        node->flags = VFS_FILE;
        node->size = f.data ? (uint32_t)f.size : 0;
        node->ops = &initrd_file_ops;
    }

    return node;
}

static int initrd_mount(fs_mount_t *mnt, const char *device) {
    (void)mnt;
    (void)device;

    return 0;
}

static int initrd_unmount(fs_mount_t *mnt) {
    (void)mnt;

    return 0;
}

static vnode_t *initrd_get_root(fs_mount_t *mnt) {
    (void)mnt;

    return initrd_vnode_create("", '5');
}

static fs_driver_t initrd_driver = {
    .name = "initrd",
    .mount = initrd_mount,
    .unmount = initrd_unmount,
    .get_root = initrd_get_root
};

fs_driver_t *initrd_fs_get_driver() {
    return &initrd_driver;
}