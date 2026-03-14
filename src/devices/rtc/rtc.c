#include "rtc.h"

rtc_time_t boot_time;
rtc_time_t curr_time;

static uint8_t cmos_read(uint8_t reg) {
    outb(CMOS_REG_SELECT, reg & 0x7f);
    outb(0x80, 0x00);

    return inb(CMOS_REG_DATA);
}

static uint8_t bcd_to_binary(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0f);
}

void rtc_read_time(rtc_time_t *t) {
    uint8_t regB;

    __asm__ volatile ("cli");

    while (cmos_read(RTC_REG_A) & RTC_UIP_FLAG);

    t->seconds = cmos_read(RTC_SECONDS);
    t->minutes = cmos_read(RTC_MINUTES);
    t->hours = cmos_read(RTC_HOURS);
    t->day = cmos_read(RTC_DAY);
    t->month = cmos_read(RTC_MONTH);
    t->year = cmos_read(RTC_YEAR);

    regB = cmos_read(RTC_REG_B);

    __asm__ volatile ("sti");

    if (!(regB & RTC_BINARY_MODE)) {
        t->seconds = bcd_to_binary(t->seconds);
        t->minutes = bcd_to_binary(t->minutes);
        t->hours = bcd_to_binary(t->hours);
        t->day = bcd_to_binary(t->day);
        t->month = bcd_to_binary(t->month);
        t->year = bcd_to_binary(t->year);
    }

    if (!(regB & RTC_24HR_MODE) && (t->hours & 0x80))
        t->hours = ((t->hours & 0x7f) + 12) % 24;
    
    t->year += 2000;
}

void rtc_init() {
    rtc_read_time(&boot_time);
}