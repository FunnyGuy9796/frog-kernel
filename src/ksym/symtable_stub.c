#include <stdint.h>

typedef struct {
    uint32_t addr;
    const char *name;
} ksym_entry_t;

ksym_entry_t ksym_entries[] = { { 0, 0 } };
const uint32_t ksym_entry_count = 0;