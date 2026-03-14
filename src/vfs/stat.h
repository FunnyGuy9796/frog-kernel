#ifndef SYS_STAT_H
#define SYS_STAT_H

#include <stdint.h>

// File type bitmask and flag definitions
#define S_IFMT   0xF000  // mask for file type bits
#define S_IFREG  0x8000  // regular file
#define S_IFDIR  0x4000  // directory
#define S_IFCHR  0x2000  // character device
#define S_IFBLK  0x6000  // block device
#define S_IFIFO  0x1000  // pipe/FIFO
#define S_IFLNK  0xA000  // symbolic link

// Permission bits
#define S_IRUSR  0400    // owner read
#define S_IWUSR  0200    // owner write
#define S_IXUSR  0100    // owner execute
#define S_IRGRP  0040    // group read
#define S_IWGRP  0020    // group write
#define S_IXGRP  0010    // group execute
#define S_IROTH  0004    // others read
#define S_IWOTH  0002    // others write
#define S_IXOTH  0001    // others execute

// Convenience macros for testing file type
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)

typedef uint32_t ino_t;
typedef uint32_t dev_t;
typedef uint32_t mode_t;
typedef uint32_t nlink_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef int32_t  off_t;
typedef int64_t  time_t;

struct stat {
    dev_t   st_dev;      // device containing the file
    ino_t   st_ino;      // inode number
    mode_t  st_mode;     // file type and permissions
    nlink_t st_nlink;    // number of hard links
    uid_t   st_uid;      // owner user ID
    gid_t   st_gid;      // owner group ID
    dev_t   st_rdev;     // device ID (for character/block devices)
    off_t   st_size;     // file size in bytes
    uint32_t st_blksize; // preferred block size for I/O
    uint32_t st_blocks;  // number of 512-byte blocks allocated
    time_t  st_atime;    // last access time
    time_t  st_mtime;    // last modification time
    time_t  st_ctime;    // last status change time
};

#endif