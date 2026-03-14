#ifndef PANIC_H
#define PANIC_H

#include "printf.h"
#include "../ksym/ksym.h"

typedef struct registers registers_t;

void panic(registers_t *regs, const char *fmt, ...);

#endif