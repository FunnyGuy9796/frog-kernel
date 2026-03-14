#ifndef INITRD_H
#define INITRD_H

#include "../multiboot.h"
#include "../misc/util.h"
#include "../misc/mem.h"
#include "../misc/panic.h"
#include "../memory/paging.h"
#include "../memory/kheap.h"

typedef struct {
    char name[100];     /* Filename */
    char mode[8];       /* Octal permissions */
    char uid[8];        /* Octal user ID */
    char gid[8];        /* Octal group ID */
    char size[12];      /* Octal file size in bytes */
    char mtime[12];     /* Octal modification time */
    char checksum[8];   /* Header checksum */
    char typeflag;      /* '0'=file, '2'=symlink, '5'=dir, etc. */
    char linkname[100]; /* Target if symlink */
    char magic[6];      /* "ustar" */
    char version[2];    /* "00" */
    char uname[32];     /* Owner username (informational) */
    char gname[32];     /* Owner group name (informational) */
    char devmajor[8];   /* Device major (for special files) */
    char devminor[8];   /* Device minor (for special files) */
    char prefix[155];   /* Path prefix for long filenames */
    char pad[12];       /* Padding to 512 bytes */
} ustar_header_t;

typedef struct {
    uint32_t addr;
    uint32_t phys_addr;
    size_t size;
} initrd_t;

typedef struct {
    char *name;
    char *data;
    size_t size;
    uint8_t typeflag;
} initrd_file_t;

extern initrd_t initrd;

static inline uint32_t oct2int(const char *str, size_t len) {
    uint32_t val = 0;

    for (size_t i = 0; i < len; i++) {
        if (str[i] < '0' || str[i] > '7')
            break;
        
        val = (val << 3) | (str[i] - '0');
    }

    return val;
}

void initrd_init(void *mb2_info);
void initrd_remap();
initrd_file_t initrd_lookup(const char *filename);

#endif