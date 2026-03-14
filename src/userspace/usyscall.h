#ifndef USYSCALL_H
#define USYSCALL_H

#include <stdint.h>
#include "../idt/idt.h"
#include "../vfs/vfs.h"
#include "uprocess.h"

#define SYS_EXIT 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_READ 4
#define SYS_WRITE 5
#define SYS_IOCTL 6
#define SYS_READDIR 7
#define SYS_FSTAT 8
#define SYS_TIME 9
#define SYS_SPAWN 10
#define SYS_WAIT 11
#define SYS_GETPID 12
#define SYS_GETPPID 13

void userspace_init();

#endif