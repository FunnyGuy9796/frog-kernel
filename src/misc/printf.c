#include "printf.h"

static int vsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
    size_t i = 0;
#define PUT(c) do { if (i < size - 1) buf[i++] = (c); } while (0)

    while (*fmt) {
        if (*fmt != '%') {
            PUT(*fmt++);
            
            continue;
        }

        fmt++;

        int left_align = 0;
        int zero_pad   = 0;

        while (*fmt == '-' || *fmt == '0') {
            if (*fmt == '-')
                left_align = 1;

            if (*fmt == '0')
                zero_pad = 1;

            fmt++;
        }
        
        if (left_align)
            zero_pad = 0;

        int width = 0;

        if (*fmt == '*') {
            width = va_arg(args, int);
            fmt++;
        } else {
            while (*fmt >= '0' && *fmt <= '9')
                width = width * 10 + (*fmt++ - '0');
        }

        int precision = -1;

        if (*fmt == '.') {
            fmt++;
            precision = 0;

            if (*fmt == '*') {
                precision = va_arg(args, int);
                fmt++;
            } else {
                while (*fmt >= '0' && *fmt <= '9')
                    precision = precision * 10 + (*fmt++ - '0');
            }
        }

#define PAD_AND_PUT(tmp, len) do {                          \
    int _len = (len);                                       \
    int _pad = width - _len;                                \
    char _pc = zero_pad ? '0' : ' ';                        \
    if (!left_align) while (_pad-- > 0) PUT(_pc);           \
    for (int _j = 0; _j < _len; _j++) PUT((tmp)[_j]);      \
    if (left_align)  while (_pad-- > 0) PUT(' ');           \
} while (0)

        switch (*fmt++) {
            case 'c': {
                char tmp[1] = { (char)va_arg(args, int) };

                PAD_AND_PUT(tmp, 1);

                break;
            }

            case 's': {
                const char *s = va_arg(args, const char *);

                if (!s)
                    s = "(null)";

                int len = 0;

                while (s[len])
                    len++;

                if (precision >= 0 && precision < len)
                    len = precision;
                    
                PAD_AND_PUT(s, len);

                break;
            }

            case 'd': {
                char tmp[32];
                int val = va_arg(args, int);
                int neg = val < 0;

                if (neg)
                    val = -val;

                utoa((unsigned int)val, tmp + neg, 10);

                if (neg)
                    tmp[0] = '-';

                int len = 0;

                while (tmp[len])
                    len++;
                
                if (precision > (len - neg)) {
                    int digits_needed = precision - (len - neg);
                    
                    for (int j = len; j >= neg; j--)
                        tmp[j + digits_needed] = tmp[j];

                    for (int j = neg; j < neg + digits_needed; j++)
                        tmp[j] = '0';

                    len += digits_needed;
                }

                PAD_AND_PUT(tmp, len);

                break;
            }

            case 'u': {
                char tmp[32];

                utoa(va_arg(args, unsigned int), tmp, 10);

                int len = 0;
                
                while (tmp[len])
                    len++;

                if (precision > len) {
                    int pad = precision - len;

                    for (int j = len; j >= 0; j--)
                        tmp[j + pad] = tmp[j];

                    for (int j = 0; j < pad; j++)
                        tmp[j] = '0';

                    len = precision;
                }

                PAD_AND_PUT(tmp, len);

                break;
            }

            case 'x': {
                char tmp[32];

                utoa(va_arg(args, unsigned int), tmp, 16);

                int len = 0; while (tmp[len]) len++;

                if (precision > len) {
                    int pad = precision - len;

                    for (int j = len; j >= 0; j--)
                        tmp[j + pad] = tmp[j];

                    for (int j = 0; j < pad; j++)
                        tmp[j] = '0';

                    len = precision;
                }

                PAD_AND_PUT(tmp, len);

                break;
            }

            case 'p': {
                char tmp[34];

                tmp[0] = '0'; tmp[1] = 'x';

                utoa((unsigned int)va_arg(args, void *), tmp + 2, 16);

                int len = 0; while (tmp[len])
                    len++;

                PAD_AND_PUT(tmp, len);

                break;
            }
            case 'f': {
                char tmp[64];

                ftoa(va_arg(args, double), tmp, precision >= 0 ? precision : 6);

                int len = 0; while (tmp[len])
                    len++;

                PAD_AND_PUT(tmp, len);

                break;
            }

            case '%': {
                PUT('%');

                break;
            }
        }
#undef PAD_AND_PUT
    }

#undef PUT
    buf[i] = '\0';

    return (int)i;
}

int ksnprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);

    int len = vsnprintf(buf, size, fmt, args);

    va_end(args);
    
    return len;
}

int serial_printf(const char *fmt, ...) {
    char buf[1024];
    va_list args;

    va_start(args, fmt);

    int len = vsnprintf(buf, sizeof(buf), fmt, args);

    va_end(args);

    serial_write(buf);

    return len;
}

int serial_vprintf(const char *fmt, va_list args) {
    char buf[1024];
    int len = vsnprintf(buf, sizeof(buf), fmt, args);

    serial_write(buf);

    return len;
}

int console_printf(const char *fmt, ...) {
    char buf[1024];
    va_list args;

    va_start(args, fmt);

    int len = vsnprintf(buf, sizeof(buf), fmt, args);

    va_end(args);

    for (char *p = buf; *p; p++)
        console_putchar(*p);

    return len;
}

int console_vprintf(const char *fmt, va_list args) {
    char buf[1024];
    int len = vsnprintf(buf, sizeof(buf), fmt, args);

    for (char *p = buf; *p; p++)
        console_putchar(*p);

    return len;
}