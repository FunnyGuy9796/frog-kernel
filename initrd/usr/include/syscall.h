#ifndef SYSCALL_H
#define SYSCALL_H

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

static inline int syscall0(int num) {
    int ret;

    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num)
        : "memory"
    );

    return ret;
}

static inline int syscall1(int num, int a) {
    int ret;

    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(a)
        : "memory"
    );
    
    return ret;
}

static inline int syscall2(int num, int a, int b) {
    int ret;

    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(a), "c"(b)
        : "memory"
    );
    
    return ret;
}

static inline int syscall3(int num, int a, int b, int c) {
    int ret;

    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(a), "c"(b), "d"(c)
        : "memory"
    );
    
    return ret;
}

static inline int syscall4(int num, int a, int b, int c, int d) {
    int ret;

    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(a), "c"(b), "d"(c), "S"(d)
        : "memory"
    );
    
    return ret;
}

#endif