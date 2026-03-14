#ifndef STDLIB_H
#define STDLIB_H

#include "syscall.h"

void itoa(int value, char *buf, int base);
void utoa(unsigned int value, char *buf, int base);
void ftoa(double value, char *buf, int decimals);

#endif