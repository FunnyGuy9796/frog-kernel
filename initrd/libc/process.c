#include "../usr/include/process.h"

pid_t spawn(const char *path, int argc, char **argv) {
    return (pid_t)syscall3(SYS_SPAWN, (int)path, (int)argv, argc);
}