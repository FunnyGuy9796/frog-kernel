#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include "../../misc/util.h"
#include "../../misc/printf.h"
#include "../../misc/panic.h"

#define CMOS_REG_SELECT 0x70
#define CMOS_REG_DATA 0x71

#define RTC_SECONDS 0x00
#define RTC_MINUTES 0x02
#define RTC_HOURS 0x04
#define RTC_DAY 0x07
#define RTC_MONTH 0x08
#define RTC_YEAR 0x09
#define RTC_REG_A 0x0a
#define RTC_REG_B 0x0b

#define RTC_UIP_FLAG 0x80
#define RTC_BINARY_MODE 0x04
#define RTC_24HR_MODE 0x02

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

extern rtc_time_t boot_time;
extern rtc_time_t curr_time;

void rtc_read_time(rtc_time_t *t);
void rtc_init();

#endif