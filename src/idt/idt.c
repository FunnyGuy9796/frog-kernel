#include "idt.h"

#define IDT_ENTRIES 256
#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xA0
#define PIC2_DATA   0xA1

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t   idt_ptr;

extern void isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7;
extern void isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15;
extern void isr16, isr17, isr18, isr19, isr128;
extern void irq0,  irq1,  irq2,  irq3,  irq4,  irq5,  irq6,  irq7;
extern void irq8,  irq9,  irq10, irq11, irq12, irq13, irq14, irq15;

static void idt_set_entry(uint8_t num, void *base, uint16_t selector, uint8_t flags) {
    idt[num].base_low  = (uint32_t)base & 0xFFFF;
    idt[num].base_high = ((uint32_t)base >> 16) & 0xFFFF;
    idt[num].selector  = selector;
    idt[num].reserved  = 0;
    idt[num].flags     = flags;
}

static void pic_remap(void) {
    outb(PIC1_CMD,  0x11);
    outb(PIC2_CMD,  0x11);

    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}

void idt_init(void) {
    memset(idt, 0, sizeof(idt));

    idt_set_entry(0,  &isr0,  0x08, 0x8e);
    idt_set_entry(1,  &isr1,  0x08, 0x8e);
    idt_set_entry(2,  &isr2,  0x08, 0x8e);
    idt_set_entry(3,  &isr3,  0x08, 0x8e);
    idt_set_entry(4,  &isr4,  0x08, 0x8e);
    idt_set_entry(5,  &isr5,  0x08, 0x8e);
    idt_set_entry(6,  &isr6,  0x08, 0x8e);
    idt_set_entry(7,  &isr7,  0x08, 0x8e);
    idt_set_entry(8,  &isr8,  0x08, 0x8e);
    idt_set_entry(9,  &isr9,  0x08, 0x8e);
    idt_set_entry(10, &isr10, 0x08, 0x8e);
    idt_set_entry(11, &isr11, 0x08, 0x8e);
    idt_set_entry(12, &isr12, 0x08, 0x8e);
    idt_set_entry(13, &isr13, 0x08, 0x8e);
    idt_set_entry(14, &isr14, 0x08, 0x8e);
    idt_set_entry(15, &isr15, 0x08, 0x8e);
    idt_set_entry(16, &isr16, 0x08, 0x8e);
    idt_set_entry(17, &isr17, 0x08, 0x8e);
    idt_set_entry(18, &isr18, 0x08, 0x8e);
    idt_set_entry(19, &isr19, 0x08, 0x8e);

    pic_remap();

    idt_set_entry(32, &irq0,  0x08, 0x8e);
    idt_set_entry(33, &irq1,  0x08, 0x8e);
    idt_set_entry(34, &irq2,  0x08, 0x8e);
    idt_set_entry(35, &irq3,  0x08, 0x8e);
    idt_set_entry(36, &irq4,  0x08, 0x8e);
    idt_set_entry(37, &irq5,  0x08, 0x8e);
    idt_set_entry(38, &irq6,  0x08, 0x8e);
    idt_set_entry(39, &irq7,  0x08, 0x8e);
    idt_set_entry(40, &irq8,  0x08, 0x8e);
    idt_set_entry(41, &irq9,  0x08, 0x8e);
    idt_set_entry(42, &irq10, 0x08, 0x8e);
    idt_set_entry(43, &irq11, 0x08, 0x8e);
    idt_set_entry(44, &irq12, 0x08, 0x8e);
    idt_set_entry(45, &irq13, 0x08, 0x8e);
    idt_set_entry(46, &irq14, 0x08, 0x8e);
    idt_set_entry(47, &irq15, 0x08, 0x8e);

    idt_set_entry(0x80, &isr128, 0x08, 0xee);

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint32_t)&idt;

    __asm__ volatile("lidt %0" :: "m"(idt_ptr));
    __asm__ volatile("sti");
}