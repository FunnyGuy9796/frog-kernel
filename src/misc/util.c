#include "util.h"

size_t strlen(const char *str) {
    size_t len = 0;

    while (str[len])
        len++;
    
    return len;
}

size_t strnlen(const char *str, size_t maxlen) {
    size_t len = 0;

    while (len < maxlen && str[len] != '\0')
        len++;
    
    return len;
}

char *strcpy(char *dst, const char *src) {
    char *p = dst;

    while ((*p++ = *src++));

    return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
    char *p = dst;

    while (n > 0 && *src) {
        *p++ = *src++;
        n--;
    }

    while (n-- > 0)
        *p++ = '\0';
    
    return dst;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }

    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    if (n == 0)
        return 0;
    
    while (--n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }

    return (unsigned char)*s1 - (unsigned char)*s2;
}

char *strchr(const char *s, int c) {
    unsigned char ch = (unsigned char)c;

    for (;;) {
        if ((unsigned char)*s == ch)
            return (char *)s;
        
        if (*s == '\0')
            return NULL;
        
        s++;
    }
}

char *strrchr(const char *s, int c) {
    unsigned char ch = (unsigned char)c;
    char *last = NULL;

    for (;;) {
        if ((unsigned char)*s == ch)
            last = (char *)s;

        if (*s == '\0')
            return last;
            
        s++;
    }
}

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