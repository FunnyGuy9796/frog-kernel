#ifndef ISR_H
#define ISR_H

#include <stdint.h>
#include "../misc/printf.h"
#include "../devices/pit/pit.h"

typedef struct registers {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_num;
    uint32_t err_code;
    uint32_t eip, cs, eflags;
} registers_t;

typedef void (*irq_handler_t)(registers_t *);
typedef void (*syscall_handler_t)(registers_t *regs);

static const char *exception_messages[] = {
    "division by zero",
    "debug",
    "non-maskable interrupt",
    "breakpoint",
    "overflow",
    "bound range exceeded",
    "invalid opcode",
    "device not available",
    "double fault",
    "coprocessor segment overrun",
    "invalid TSS",
    "segment not present",
    "stack segment fault",
    "general protection fault",
    "page fault",
    "reserved",
    "x87 floating point exception",
    "alignment check",
    "machine check",
    "SIMD floating point exception"
};

void exception_handler(registers_t *regs);
void irq_register(uint8_t irq, irq_handler_t handler);
void syscall_register(uint8_t num, syscall_handler_t handler);
void irq_handler(registers_t *regs);

#endif