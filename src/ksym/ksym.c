#include "ksym.h"

void ksym_init(void) {
    serial_printf("ksym_init: %u symbols loaded\n", ksym_entry_count);
    
    if (ksym_entry_count > 0) {
        serial_printf("  first: 0x%08x %s\n", ksym_entries[0].addr, ksym_entries[0].name);
        serial_printf("  last:  0x%08x %s\n", ksym_entries[ksym_entry_count - 1].addr, ksym_entries[ksym_entry_count - 1].name);
    }
}

const char *ksym_lookup(uint32_t eip, uint32_t *offset_out, uint32_t *size_out) {
    if (ksym_entry_count == 0)
        return "??";

    const ksym_entry_t *best = NULL;

    for (uint32_t i = 0; i < ksym_entry_count; i++) {
        if (ksym_entries[i].addr > eip)
            break;
        
        best = &ksym_entries[i];
    }

    if (!best)
        return "??";

    if (offset_out)
        *offset_out = eip - best->addr;

    if (size_out) {
        uint32_t idx = best - ksym_entries;

        *size_out = (idx + 1 < ksym_entry_count) ? ksym_entries[idx + 1].addr - best->addr : 0;
    }

    return best->name;
}

char *ksym_format(uint32_t eip, char *buf, uint32_t buflen) {
    uint32_t offset = 0, size = 0;
    const char *name = ksym_lookup(eip, &offset, &size);

    if (size > 0)
        ksnprintf(buf, buflen, "%s+0x%x/0x%x", name, offset, size);
    else
        ksnprintf(buf, buflen, "%s+0x%x", name, offset);

    return buf;
}

void ksym_stacktrace(uint32_t ebp, uint32_t eip, uint32_t max_frames) {
    char buf[64];

    serial_printf("stack trace:\n");
    serial_printf("  [0] 0x%08x  %s\n", eip, ksym_format(eip, buf, sizeof(buf)));

    for (uint32_t i = 1; i < max_frames; i++) {
        if (ebp == 0 || ebp < 0xc0000000)
            break;
        
        uint32_t ret       = *(uint32_t *)(ebp + 4);
        uint32_t saved_ebp = *(uint32_t *)(ebp);
        
        if (ret == 0) break;
        
        serial_printf("  [%u] 0x%08x  %s\n", i, ret, ksym_format(ret, buf, sizeof(buf)));
        
        if (saved_ebp <= ebp)
            break;

        ebp = saved_ebp;
    }
}