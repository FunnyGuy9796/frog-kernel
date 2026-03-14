#include "../usr/include/stdlib.h"
#include "../usr/include/syscall.h"

void itoa(int value, char *buf, int base) {
    char tmp[32];
    char digits[] = "0123456789abcdef";
    int i = 0;
    int neg = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        
        return;
    }

    if (value < 0 && base == 10) {
        neg = 1;
        value = -value;
    }

    while (value > 0) {
        tmp[i++] = digits[value % base];
        value /= base;
    }

    if (neg)
        tmp[i++] = '-';
    
    int j = 0;

    while (i > 0)
        buf[j++] = tmp[--i];
    
    buf[j] = '\0';
}

void utoa(unsigned int value, char *buf, int base) {
    char tmp[32];
    char digits[] = "0123456789abcdef";
    int i = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';

        return;
    }

    while (value > 0) {
        tmp[i++] = digits[value % base];
        value /= base;
    }

    int j = 0;

    while (i > 0)
        buf[j++] = tmp[--i];
    
    buf[j] = '\0';
}

void ftoa(double value, char *buf, int decimals) {
    int i = 0;

    if (value < 0) {
        buf[i++] = '-';
        value = -value;
    }

    int int_part = (int)value;
    double frac_part = value - int_part;
    char tmp[32];

    itoa(int_part, tmp, 10);

    for (char *p = tmp; *p; p++)
        buf[i++] = *p;
    
    if (decimals <= 0) {
        buf[i] = '\0';

        return;
    }

    buf[i++] = '.';

    for (int d = 0; d < decimals; d++) {
        frac_part *= 10;

        int digit = (int)frac_part;

        buf[i++] = '0' + digit;
        frac_part -= digit;
    }

    buf[i] = '\0';
}