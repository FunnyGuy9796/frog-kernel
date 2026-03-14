#ifndef INITRD_FS_H
#define INITRD_FS_H

#include "../misc/printf.h"
#include "initrd.h"
#include "../vfs/vfs.h"

fs_driver_t *initrd_fs_get_driver();

#endif