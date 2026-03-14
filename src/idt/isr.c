#include "isr.h"

static irq_handler_t irq_handlers[16] = {0};
static syscall_handler_t syscall_handlers[256] = {0};

void exception_handler(registers_t *regs) {
    static registers_t saved_regs;
    saved_regs = *regs;

    registers_t *r = &saved_regs;

    if (r->int_num == 0x80) {
        uint8_t num = r->eax & 0xff;

        if (syscall_handlers[num])
            syscall_handlers[num](regs);
        else
            serial_printf("unhandled syscall %d\n", num);
        
        return;
    }

    __asm__ volatile ("cli");

    uint32_t cr0, cr2, cr3, cr4, ss;

    __asm__ volatile (
        "mov %%cr0, %0\n"
        "mov %%cr2, %1\n"
        "mov %%cr3, %2\n"
        "mov %%cr4, %3\n"
        : "=r"(cr0), "=r"(cr2),
          "=r"(cr3), "=r"(cr4)
    );

    if (r->int_num == 14) {
        serial_printf("\npage fault at 0x%08x\n", cr2);
        serial_printf("  %s | %s | %s%s\n",
            r->err_code & 1 ? "protection-violation" : "not-present",
            r->err_code & 2 ? "write"                : "read",
            r->err_code & 4 ? "user"                 : "kernel",
            r->err_code & 8 ? " | reserved-bit-set"  : "");
    }

    char sym_buf[64];

    serial_printf("\nexception %d: %s\n  err=0x%08x\n", r->int_num, r->int_num < 20 ? exception_messages[r->int_num] : "unkown", r->err_code);
    serial_printf("  eip=0x%08x  eflags: 0x%08x\n", r->eip, r->eflags);
    serial_printf("  eax=0x%08x  ebx=0x%08x  ecx=0x%08x  edx=0x%08x\n", r->eax, r->ebx, r->ecx, r->edx);
    serial_printf("  esi=0x%08x  edi=0x%08x  ebp=0x%08x  esp=0x%08x\n", r->esi, r->edi, r->ebp, r->esp);
    serial_printf("  cs=0x%04x  ds=0x%04x  es=0x%04x\n", r->cs, r->ds, r->es);
    serial_printf("  fs=0x%04x  gs=0x%04x  ss=0x%04x\n", r->fs, r->gs, ss);
    serial_printf("  cr0=0x%08x  cr2=0x%08x  cr3=0x%08x  cr4=0x%08x\n", cr0, cr2, cr3, cr4);

    serial_printf("\n  eflags: 0x%08x\n", r->eflags);
    serial_printf("  cf=%d pf=%d af=%d zf=%d sf=%d tf=%d if=%d df=%d of=%d iopl=%d\n",
        (r->eflags >>  0) & 1,
        (r->eflags >>  2) & 1,
        (r->eflags >>  4) & 1,
        (r->eflags >>  6) & 1,
        (r->eflags >>  7) & 1,
        (r->eflags >>  8) & 1,
        (r->eflags >>  9) & 1,
        (r->eflags >> 10) & 1,
        (r->eflags >> 11) & 1,
        (r->eflags >> 12) & 3);

    serial_printf("\n  cr0: 0x%08x\n", cr0);
    serial_printf("  pe=%d mp=%d em=%d ts=%d wp=%d pg=%d\n",
        (cr0 >> 0) & 1,
        (cr0 >> 1) & 1,
        (cr0 >> 2) & 1,
        (cr0 >> 3) & 1,
        (cr0 >> 16) & 1,
        (cr0 >> 31) & 1);
    
    serial_printf("\n  stack (esp=0x%08x):\n", r->esp);

    uint32_t *sp = (uint32_t *)r->esp;

    for (int i = 0; i < 8; i++) {
        if ((uint32_t)(sp + i) < KERNEL_OFFSET)
            break;

        uint32_t val = sp[i];

        if (val >= KERNEL_OFFSET) {
            const char *stack_sym = ksym_lookup(val, NULL, NULL);

            if (stack_sym[0] != '?')
                serial_printf("  esp+%02d: 0x%08x  <%s>\n", i * 4, val, ksym_format(val, sym_buf, sizeof(sym_buf)));
            else
                serial_printf("  esp+%02d: 0x%08x\n", i * 4, val);
        } else
            serial_printf("  esp+%02d: 0x%08x\n", i * 4, val);
    }

    serial_printf("\n  stack trace:\n");
    serial_printf("  [0] 0x%08x  <%s>\n", r->eip, ksym_format(r->eip, sym_buf, sizeof(sym_buf)));

    uint32_t ebp = r->ebp;

    for (int i = 1; i < 16; i++) {
        if (ebp == 0 || ebp < KERNEL_OFFSET)
            break;

        uint32_t ret = *(uint32_t *)(ebp + 4);
        uint32_t saved_ebp = *(uint32_t *)(ebp);

        if (ret == 0)
        break;

        serial_printf("  [%d] 0x%08x  <%s>\n", i, ret, ksym_format(ret, sym_buf, sizeof(sym_buf)));

        if (saved_ebp <= ebp)
            break;
            
        ebp = saved_ebp;
    }

    serial_printf("\n  uptime: %u ticks (%u ms)\n", pit_get_ticks(), pit_get_ticks() * (1000 / PIT_HZ));

    panic(r, "exception_handler() -> an exception occurred");
}

void irq_register(uint8_t irq, irq_handler_t handler) {
    irq_handlers[irq] = handler;
}

void syscall_register(uint8_t num, syscall_handler_t handler) {
    syscall_handlers[num] = handler;
}

void irq_handler(registers_t *regs) {
    uint8_t irq = regs->int_num - 32;

    if (irq_handlers[irq])
        irq_handlers[irq](regs);

    if (regs->int_num >= 40)
        outb(0xa0, 0x20);
    
    outb(0x20, 0x20);
}