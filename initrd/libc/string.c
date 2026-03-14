#include "../usr/include/string.h"

void *memset(void *dst, int c, size_t n) {
    uint8_t *p = (uint8_t *)dst;

    while (n--)
        *p++ = (uint8_t)c;
    
    return dst;
}

void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;

    while (n--)
        *d++ = *s++;
    
    return dst;
}

void *memmove(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;

    if (d < s) {
        while (n--)
            *d++ = *s++;
    } else {
        d += n;
        s += n;

        while (n--)
            *--d = *--s;
    }

    return dst;
}

int memcmp(const void *a, const void *b, size_t n) {
    const uint8_t *p = (const uint8_t *)a;
    const uint8_t *q = (const uint8_t *)b;

    while (n--) {
        if (*p != *q)
            return *p - *q;
        
        p++;
        q++;
    }

    return 0;
}

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