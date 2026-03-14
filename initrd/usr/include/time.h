#ifndef TIME_H
#define TIME_H

#include <stdint.h>
#include "syscall.h"

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} time_t;

int gettimeofday(time_t *t) {
    return syscall1(SYS_TIME, (int)t);
}

#endif