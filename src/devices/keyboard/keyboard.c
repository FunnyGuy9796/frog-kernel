#include "keyboard.h"
#include "../../idt/isr.h"

volatile uint8_t keyboard_dirty = 0;
keyboard_state_t keyboard;
keyboard_buf_t keyboard_buf = { 0 };

static uint8_t extended = 0;

static const char scancode_map[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static const char scancode_map_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

void tty_input_push(char c);

static void buf_push(uint8_t c, uint8_t released) {
    uint32_t next = (keyboard_buf.head + 1) % KEYBOARD_BUF_SIZE;

    if (next == keyboard_buf.tail)
        return;
    
    keyboard_buf.buf[keyboard_buf.head] = released ? (c | KEY_RELEASED) : c;
    keyboard_buf.head = next;
}

static int buf_pop(uint8_t *c) {
    if (keyboard_buf.head == keyboard_buf.tail)
        return 0;
    
    *c = keyboard_buf.buf[keyboard_buf.tail];
    keyboard_buf.tail = (keyboard_buf.tail + 1) % KEYBOARD_BUF_SIZE;

    return 1;
}

static int keyboard_read_wrap(uint8_t *buf, uint32_t offset, uint32_t len) {
    (void)offset;

    uint32_t i = 0;

    while (i < len) {
        uint8_t raw;

        if (!buf_pop(&raw))
            break;
        
        buf[i++] = raw;
    }

    return (int)i;
}

static int keyboard_write_wrap(const uint8_t *buf, uint32_t offset, uint32_t len) {
    (void)buf;
    (void)offset;
    (void)len;

    return -1;
}

void keyboard_init() {
    irq_register(1, keyboard_callback);
}

void keyboard_callback(registers_t *regs) {
    uint8_t scancode = inb(0x60);

    if (scancode == 0xe0) {
        extended = 1;

        return;
    }

    uint8_t released = scancode & KEY_RELEASED;

    scancode &= ~KEY_RELEASED;

    if (extended) {
        extended = 0;

        switch (scancode) {
            case 0x1d: {
                keyboard.mods = released ? keyboard.mods & ~R_CTRL : keyboard.mods | R_CTRL;

                return;
            }

            case 0x38: {
                keyboard.mods = released ? keyboard.mods & ~R_ALT : keyboard.mods | R_ALT;

                return;
            }
        }

        return;
    }

    switch (scancode) {
        case 0x2a: {
            keyboard.mods = released ? keyboard.mods & ~L_SHIFT : keyboard.mods | L_SHIFT;

            return;
        }

        case 0x36: {
            keyboard.mods = released ? keyboard.mods & ~R_SHIFT : keyboard.mods | R_SHIFT;

            return;
        }

        case 0x1d: {
            keyboard.mods = released ? keyboard.mods & ~L_CTRL : keyboard.mods | L_CTRL;

            return;
        }

        case 0x38: {
            keyboard.mods = released ? keyboard.mods & ~L_ALT : keyboard.mods | L_ALT;

            return;
        }

        default:
            break;
    }

    if (scancode >= sizeof(scancode_map))
        return;
    
    char c = (keyboard.mods & (L_SHIFT | R_SHIFT)) ? scancode_map_shift[scancode] : scancode_map[scancode];

    if (c == 0)
        return;

    buf_push((uint8_t)c, released);

    keyboard_dirty = 1;

    if (!released)
        tty_input_push(c);
}