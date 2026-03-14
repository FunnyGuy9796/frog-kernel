#include "pit.h"
#include "../../idt/isr.h"

static volatile uint32_t ticks = 0;
static const uint8_t days_in_month[] = {
    0,  // padding so index 1 = January
    31, // January
    28, // February (non-leap)
    31, // March
    30, // April
    31, // May
    30, // June
    31, // July
    31, // August
    30, // September
    31, // October
    30, // November
    31  // December
};

static inline int leap_year(uint32_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static inline uint8_t get_days_in_month(uint8_t month, uint32_t year) {
    if (month == 2 && leap_year(year))
        return 29;
    
    return days_in_month[month];
}

void pit_init() {
    uint32_t divisor = 1193180 / PIT_HZ;

    outb(0x43, 0x36);
    outb(0x40, divisor & 0xff);
    outb(0x40, (divisor >> 8) & 0xff);

    irq_register(0, pit_callback);
}

void pit_callback(registers_t *regs) {
    ticks++;

    if (unlikely(ticks % PIT_HZ == 0))
        curr_time.seconds++;
    
    if (curr_time.seconds >= 60) {
        curr_time.minutes++;
        curr_time.seconds = 0;
    }

    if (curr_time.minutes >= 60) {
        curr_time.hours++;
        curr_time.minutes = 0;
    }

    if (curr_time.hours >= 24) {
        curr_time.day++;
        curr_time.hours = 0;
    }

    if (curr_time.day > get_days_in_month(curr_time.month, curr_time.year)) {
        curr_time.month++;
        curr_time.day = 1;

        if (curr_time.month > 12) {
            curr_time.year++;
            curr_time.month = 1;
        }
    }

    if (ticks % 500 == 0)
        console_toggle_cursor();

    if (ticks % 16 == 0)
        fb_swap();

    if (curr_thread && ticks % 10 == 0)
        schedule_pending = 1;
}

uint32_t pit_get_ticks() {
    return ticks;
}