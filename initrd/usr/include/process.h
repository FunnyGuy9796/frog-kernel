#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "syscall.h"

typedef int pid_t;

static inline void exit(int code) {
    syscall1(SYS_EXIT, code);
}

static inline int wait(pid_t pid, int *exit_code) {
    return syscall2(SYS_WAIT, (int)pid, (int)exit_code);
}

static inline pid_t getpid() {
    return (pid_t)syscall0(SYS_GETPID);
}

static inline pid_t getppid() {
    return (pid_t)syscall0(SYS_GETPPID);
}

pid_t spawn(const char *path, int argc, char **argv);

#endif