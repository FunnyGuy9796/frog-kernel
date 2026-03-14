#ifndef STAT_H
#define STAT_H

#include <stdint.h>
#include "../syscall.h"

static inline int fstat(int fd, struct stat *st) {
    return syscall2(SYS_FSTAT, fd, (int)st);
}

#endif