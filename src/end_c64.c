#include <stdint.h>
#include <string.h>

#include "end.h"
#include "sys_c64.h"
#include "input.h"

static const uint8_t MENU_LINE[] = { 138, 139, 140, 141, 32, 32, 32, 146, 147, 148, 149, 150, 151, 152 };

static void display_menu() {
    uint8_t* const linestart = screenmem + (40 * 24) + 4;
    memcpy(linestart + 18, MENU_LINE, 14);
} 

enum EndMenuState get_end_state() {
    uint8_t* linestart = screenmem + (40 * 24) + 4;
    enum InputValue input;
    enum EndMenuState state = AGAIN;

    display_menu();

    do {
        if (state == MENU) {
            linestart[16] = 93;
            linestart[17] = 108;
            linestart[23] = 32;
            linestart[24] = 32;
        } else {
            linestart[16] = 32;
            linestart[17] = 32;
            linestart[23] = 93;
            linestart[24] = 108;
        }

        input = input_next();

        if (input == LEFT || input == RIGHT)
            state ^= 1;    

    } while(input != BUTTON);

    // Clear the last row, because the interrupts stop and the last line
    // is enhanced background colour mode
    memset(screenmem + (40 * 24) + 4, 32, 32);

    return state;
}
