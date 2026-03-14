#ifndef KSYM_H
#define KSYM_H

#include <stdint.h>
#include "../misc/panic.h"

typedef struct {
    uint32_t addr;
    const char *name;
} ksym_entry_t;

extern ksym_entry_t ksym_entries[];
extern const uint32_t ksym_entry_count;

void ksym_init(void);
const char *ksym_lookup(uint32_t eip, uint32_t *offset_out, uint32_t *size_out);
char *ksym_format(uint32_t eip, char *buf, uint32_t buflen);
void ksym_stacktrace(uint32_t ebp, uint32_t eip, uint32_t max_frames);

#endif