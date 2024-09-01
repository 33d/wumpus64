#include <stdint.h>
#include <string.h>

#include "mainmenu.h"
#include "input.h"
#include "sys_c64.h"
#include <c64.h>

static uint8_t MESSAGES[][2] = {
    { (472 / 8) + 64, 5},
    { (792 / 8) + 64, 4},
    { (824 / 8) + 64, 3},
};

static void draw_screen() {
    uint_least8_t i, j, x, y;
    uint8_t* ptr;

    // Plain text mode
    VIC.ctrl1 = 0x1b;
    VIC.ctrl2 = 0x08;

    // Character memory at $3800
    VIC.addr |= 0xE;

    // Set colours
    VIC.bordercolor = COLOR_GRAY1;
    // Default background colour
    VIC.bgcolor0 = COLOR_GRAY3;

    memset(screenmem, 32, 25 * 40);
    memset(COLOR_RAM, COLOR_GRAY1, 25 * 40);

    i = 192;
    ptr = screenmem + (40 * 6) + 14;
    for (y = 0; y < 160; y += 30) {
        for (x = 0; x < 10; x++)
            ptr[y++] = i++;
    }

    ptr = screenmem + (40 * 11) + 18;
    for (y = 0; y < 3; ++y) {
        x = MESSAGES[y][1];
        j = MESSAGES[y][0];
        for (i = 0; i < x; i++)
            ptr[i] = j++;
        ptr += 40;
    }
}

enum Difficulty main_menu_get_selection() {
    uint_least8_t choice = 0, i;
    enum InputValue input;
    uint8_t* ptr;

    draw_screen();

    do {
        ptr = screenmem + (40 * 11) + 15;
        for (i = 0; i < 3; i++) {
            if (i == choice) {
                ptr[0] = 93;
                ptr[1] = 108;
            } else {
                ptr[0] = 32;
                ptr[1] = 32;
            }
            ptr += 40;
        }

        input = input_next();

        if (input == UP)
            choice = choice == 0 ? 2 : choice - 1;
        if (input == DOWN)
            choice = choice == 2 ? 0 : choice + 1;

    } while(input != BUTTON);

    return choice;
}
