#include "input.h"
#include <joystick.h>
#include <stdint.h>
#include <stdbool.h>
#include <c64.h>
#include <6502.h>

void input_init() {
    joy_install(joy_static_stddrv);

    // Use CIA timer B for debouncing:
    //  0: TOD clock doesn't matter
    //  00: Count clock ticks
    //  0: Don't load
    //  1: One shot mode
    //  1: Output goes low on underflow
    //  0: Don't output the timer yet
    //  0: Stop timer
    CIA1.crb = 0x0C;
    // Set the timer
    CIA1.tb_hi = 0xFF;
    CIA1.tb_lo = 0xFF;
}

static unsigned char prev_value;

enum InputValue input_next() {
    unsigned char value;

    // If the debounce timer is running, exit
    SEI();
    value = CIA1.crb;
    CLI();
    if (value & 0x1)
        return NONE;

    value = joy_read(JOY_2);
    // If the value hasn't changed. exit
    if (value == prev_value)
        return NONE;

    prev_value = value;
    // Restart the debounce timer
    SEI();
    CIA1.crb |= 1;
    CLI();

    if (JOY_UP(value))
        return UP;
    if (JOY_DOWN(value))
        return DOWN;
    if (JOY_LEFT(value))
        return LEFT;
    if (JOY_RIGHT(value))
        return RIGHT;
    if (JOY_BTN_1(value))
        return BUTTON;
    return CENTER;
}
