#include "panic.h"
#include "../idt/isr.h"

void panic(registers_t *regs, const char *fmt, ...) {
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

    va_list args;
    
    cursor_x = 0;
    cursor_y = 0;

    fb_clear(console_bg);

    va_start(args, fmt);

    console_vprintf(fmt, args);

    va_end(args);

    if (regs) {
        if (regs->int_num == 14) {
            console_printf("\npage fault at 0x%08x\n", cr2);
            console_printf("  %s | %s | %s%s",
                regs->err_code & 1 ? "protection-violation" : "not-present",
                regs->err_code & 2 ? "write"                : "read",
                regs->err_code & 4 ? "user"                 : "kernel",
                regs->err_code & 8 ? " | reserved-bit-set"  : "");
        }

        char sym_buf[64];

        console_printf("\n\nexception %d: %s\n  err=0x%08x\n", regs->int_num, regs->int_num < 20 ? exception_messages[regs->int_num] : "unkown", regs->err_code);
        console_printf("  eip=0x%08x  eflags=0x%08x\n", regs->eip, regs->eflags);
        console_printf("  eax=0x%08x  ebx=0x%08x  ecx=0x%08x  edx=0x%08x\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
        console_printf("  esi=0x%08x  edi=0x%08x  ebp=0x%08x  esp=0x%08x\n", regs->esi, regs->edi, regs->ebp, regs->esp);
        console_printf("  cs=0x%04x  ds=0x%04x  es=0x%04x\n", regs->cs, regs->ds, regs->es);
        console_printf("  fs=0x%04x  gs=0x%04x  ss=0x%04x\n", regs->fs, regs->gs, ss);
        console_printf("  cr0=0x%08x  cr2=0x%08x  cr3=0x%08x  cr4=0x%08x\n", cr0, cr2, cr3, cr4);

        console_printf("\n  eflags: 0x%08x\n", regs->eflags);
        console_printf("  cf=%d pf=%d af=%d zf=%d sf=%d tf=%d if=%d df=%d of=%d iopl=%d\n",
            (regs->eflags >>  0) & 1,
            (regs->eflags >>  2) & 1,
            (regs->eflags >>  4) & 1,
            (regs->eflags >>  6) & 1,
            (regs->eflags >>  7) & 1,
            (regs->eflags >>  8) & 1,
            (regs->eflags >>  9) & 1,
            (regs->eflags >> 10) & 1,
            (regs->eflags >> 11) & 1,
            (regs->eflags >> 12) & 3);
        
        console_printf("\n  cr0: 0x%08x\n", cr0);
        console_printf("  pe=%d mp=%d em=%d ts=%d wp=%d pg=%d\n",
            (cr0 >> 0) & 1,
            (cr0 >> 1) & 1,
            (cr0 >> 2) & 1,
            (cr0 >> 3) & 1,
            (cr0 >> 16) & 1,
            (cr0 >> 31) & 1);

        console_printf("\n  stack (esp=0x%08x):\n", regs->esp);

        uint32_t *sp = (uint32_t *)regs->esp;

        for (int i = 0; i < 8; i++) {
            if ((uint32_t)(sp + i) < KERNEL_OFFSET)
                break;

            uint32_t val = sp[i];

            if (val >= KERNEL_OFFSET) {
                const char *stack_sym = ksym_lookup(val, NULL, NULL);

                if (stack_sym[0] != '?')
                    console_printf("  esp+%02d: 0x%08x  <%s>\n", i * 4, val, ksym_format(val, sym_buf, sizeof(sym_buf)));
                else
                    console_printf("  esp+%02d: 0x%08x\n", i * 4, val);
            } else
                console_printf("  esp+%02d: 0x%08x\n", i * 4, val);
        }

        console_printf("\n  stack trace:\n");
        console_printf("  [0] 0x%08x  <%s>\n", regs->eip, ksym_format(regs->eip, sym_buf, sizeof(sym_buf)));

        uint32_t ebp = regs->ebp;

        for (int i = 1; i < 16; i++) {
            if (ebp == 0 || ebp < KERNEL_OFFSET)
                break;

            uint32_t ret = *(uint32_t *)(ebp + 4);
            uint32_t saved_ebp = *(uint32_t *)(ebp);

            if (ret == 0)
            break;

            console_printf("  [%d] 0x%08x  <%s>\n", i, ret, ksym_format(ret, sym_buf, sizeof(sym_buf)));

            if (saved_ebp <= ebp)
                break;
                
            ebp = saved_ebp;
        }

        console_printf("\n  uptime: %u ticks (%u ms)\n", pit_get_ticks(), pit_get_ticks() * (1000 / PIT_HZ));
    }

    fb_swap();

    __asm__ volatile ("hlt");

    while (1) {}
}