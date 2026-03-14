#ifndef PIT_H
#define PIT_H

#include <stdint.h>
#include "../../misc/util.h"
#include "../../framebuffer/framebuffer.h"
#include "../../thread/kthread.h"
#include "../rtc/rtc.h"

#define PIT_HZ 1000
#define PIT_MS_PER_TICK (1000 / PIT_HZ)

typedef struct registers registers_t;

void pit_init();
void pit_callback(registers_t *regs);
uint32_t pit_get_ticks();

static inline uint32_t pit_ticks_to_ms(uint32_t ticks) {
    return ticks * PIT_MS_PER_TICK;
}

static inline uint32_t pit_ms_to_ticks(uint32_t ms) {
    return ms / PIT_MS_PER_TICK;
}

static inline uint32_t pit_get_ms() {
    return pit_get_ticks() * PIT_MS_PER_TICK;
}

#endif