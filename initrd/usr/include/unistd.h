#ifndef UNISTD_H
#define UNISTD_H

#include <stddef.h>
#include "syscall.h"

#define O_RDONLY 0x00
#define O_WRONLY 0x01
#define O_RDWR 0x02
#define O_CREAT 0x40
#define O_TRUNC 0x200
#define O_APPEND 0x400

static inline int open(const char *path, int flags) {
    return syscall2(SYS_OPEN, (int)path, flags);
}

static inline int read(int fd, void *buf, int len) {
    return syscall3(SYS_READ, fd, (int)buf, len);
}

static inline int write(int fd, const void *buf, int len) {
    return syscall3(SYS_WRITE, fd, (int)buf, len);
}

static inline int close(int fd) {
    return syscall1(SYS_CLOSE, fd);
}

#endif