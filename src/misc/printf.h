#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>
#include "util.h"
#include "../serial/serial.h"
#include "../framebuffer/console.h"

int ksnprintf(char *buf, size_t size, const char *fmt, ...);
int serial_printf(const char *fmt, ...);
int serial_vprintf(const char *fmt, va_list args);
int console_printf(const char *fmt, ...);
int console_vprintf(const char *fmt, va_list args);

#endif