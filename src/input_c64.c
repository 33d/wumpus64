#include "input.h"
#include <joystick.h>
#include <stdbool.h>
#include <c64.h>

void input_init() {
    joy_install(joy_static_stddrv);
}

enum InputValue input_next() {
    unsigned char value1, value2;
    enum InputValue r;
    
    // Wait for the joystick to be centred
    do {
        value1 = joy_read(JOY_2);
        value2 = joy_read(JOY_2);
    } while (value1 != value2 ||
        value1 & (JOY_UP_MASK | JOY_DOWN_MASK | JOY_LEFT_MASK | JOY_RIGHT_MASK | JOY_BTN_1_MASK)
    );

    while (1) {
        r = NONE;
        value1 = joy_read(JOY_2);
        value2 = joy_read(JOY_2);
        if (value1 != value2)
            continue;

        if (JOY_UP(value1))
            return UP;
        if (JOY_DOWN(value1))
            return DOWN;
        if (JOY_LEFT(value1))
            return LEFT;
        if (JOY_RIGHT(value1))
            return RIGHT;
        if (JOY_BTN_1(value1))
            return BUTTON;
    }
}
