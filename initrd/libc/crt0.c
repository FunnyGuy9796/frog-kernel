#include <stdint.h>
#include <unistd.h>
#include <process.h>

extern uint32_t __bss_start;
extern uint32_t __bss_end;

int main(int argc, char **argv);

__attribute__((noreturn))
void _start_c(int argc, char **argv) {
    uint32_t *bss = &__bss_start;

    while (bss < &__bss_end)
        *bss++ = 0;
    
    open("/dev/tty0", 0);
    open("/dev/tty0", 0);
    open("/dev/tty0", 0);

    int code = main(argc, argv);

    close(0);
    close(1);
    close(2);
    
    exit(code);

    while (1)
        __asm__ volatile ("hlt");
}

__attribute__((section(".text.start")))
__attribute__((noreturn))
__attribute__((naked))
void _start() {
    __asm__ volatile (
        "mov  (%%esp), %%esi\n"
        "mov  4(%%esp), %%edi\n"
        "and  $0xfffffff0, %%esp\n"
        "sub  $4, %%esp\n"
        "push %%edi\n"
        "push %%esi\n"
        "call _start_c\n"
        ::: "esi", "edi", "memory"
    );

    __builtin_unreachable();
}