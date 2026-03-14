#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include "isr.h"
#include "../misc/util.h"

typedef struct __attribute__((packed)) {
    uint16_t base_low;
    uint16_t selector;
    uint8_t  reserved;
    uint8_t  flags;
    uint16_t base_high;
} idt_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} idt_ptr_t;

void idt_init();

#endif