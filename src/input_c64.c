#include "input.h"
#include <joystick.h>
#include <stdbool.h>
#include <c64.h>

void input_init() {
    joy_install(joy_static_stddrv);
}

enum InputValue input_next() {
    unsigned char value;
    enum InputValue r;
    
    // Wait for the joystick to be centred
    do {
        value = joy_read(JOY_2);
    } while (value & (JOY_UP_MASK | JOY_DOWN_MASK | JOY_LEFT_MASK | JOY_RIGHT_MASK | JOY_BTN_1_MASK));

    while (1) {
        r = NONE;
        value = joy_read(JOY_2);

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
    }
}
