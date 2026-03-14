#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include "../misc/util.h"

#define COM1 0x3f8

void serial_init();
void serial_putchar(char c);
void serial_write(const char *str);

#endif