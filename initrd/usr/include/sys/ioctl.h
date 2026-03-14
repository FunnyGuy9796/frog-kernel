#ifndef IOCTL_H
#define IOCTL_H

#include "../syscall.h"

static inline int ioctl(int fd, int request, void *arg) {
    return syscall3(SYS_IOCTL, fd, request, (int)arg);
}

#endif