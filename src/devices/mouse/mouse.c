#include "mouse.h"
#include "../../idt/isr.h"

mouse_state_t mouse = { 0 };

static int32_t mouse_x = 0;
static int32_t mouse_y = 0;
static uint8_t mouse_packet[3];
static uint8_t mouse_cycle = 0;

static void ps2_wait_write() {
    while (inb(0x64) & 0x02);
}

static void ps2_wait_read() {
    while (!(inb(0x64) & 0x01));
}

static void mouse_write(uint8_t data) {
    ps2_wait_write();
    outb(0x64, 0xd4);
    ps2_wait_write();
    outb(0x60, data);
}

static uint8_t mouse_read() {
    ps2_wait_read();

    return inb(0x60);
}

static void mouse_set_pos(int32_t x, int32_t y) {
    mouse_x = x;
    mouse_y = y;
}

void mouse_init() {
    ps2_wait_write();
    outb(0x64, 0xa8);

    ps2_wait_write();
    outb(0x64, 0x20);
    ps2_wait_read();

    uint8_t config = inb(0x60);
    config |= 0x02;
    config &= ~0x20;

    ps2_wait_write();
    outb(0x64, 0x60);

    ps2_wait_write();
    outb(0x60, config);

    mouse_write(0xf6);
    mouse_read();

    mouse_write(0xf4);
    mouse_read();

    irq_register(12, mouse_callback);

    mouse_set_pos(fb.width / 2, fb.height / 2);
}

void mouse_callback(registers_t *regs) {
    uint8_t data = inb(0x60);

    switch (mouse_cycle) {
        case 0: {
            mouse_packet[0] = data;

            if (data & 0x08)
                mouse_cycle++;
            
            break;
        }

        case 1: {
            mouse_packet[1] = data;
            mouse_cycle++;

            break;
        }

        case 2: {
            mouse_packet[2] = data;
            mouse_cycle = 0;

            int32_t dx = mouse_packet[1] - ((mouse_packet[0] << 4) & 0x100);
            int32_t dy = mouse_packet[2] - ((mouse_packet[0] << 3) & 0x100);

            mouse_x += dx;
            mouse_y -= dy;

            if (mouse_x < 0)
                mouse_x = 0;
            
            if (mouse_y < 0)
                mouse_y = 0;

            if (mouse_x >= fb.width)
                mouse_x = fb.width - 1;
            
            if (mouse_y >= fb.height)
                mouse_y = fb.height - 1;
            
            bool left = mouse_packet[0] & 0x01;
            bool right = mouse_packet[0] & 0x02;
            bool middle = mouse_packet[0] & 0x04;

            mouse.left_clicked = left && !mouse.left;
            mouse.right_clicked = right && !mouse.right;
            mouse.middle_clicked = middle && !mouse.middle;

            mouse.left = left;
            mouse.right = right;
            mouse.middle = middle;

            mouse.x = mouse_x;
            mouse.y = mouse_y;
        }
    }
}