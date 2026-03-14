#ifndef DIRENT_H
#define DIRENT_H

#include "syscall.h"

static inline int readdir(int fd, int index, char *name_out, int name_max) {
    return syscall4(SYS_READDIR, fd, index, (int)name_out, name_max);
}

#endif