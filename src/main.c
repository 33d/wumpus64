#include <stdint.h>
#include <stdio.h>

#include "game.h"
#include "display.h"
#include "input.h"

void main(void) {
    enum InputValue input;

    display_init();
    input_init();

    game_new(128);

    display_draw_map();

    while(true) {
        display_update_player();
        display_update_bats();

        input = input_next();
        switch (input) {
            case UP: game_move_up(); break;
            case DOWN: game_move_down(); break;
            case LEFT: game_move_left(); break;
            case RIGHT: game_move_right(); break;
        }
    }
}
