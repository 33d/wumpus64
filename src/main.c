#include <stdint.h>
#include <stdio.h>

#include "game.h"
#include "display.h"
#include "input.h"

static void new_game() {
    game_new(128);
    display_new();
}

void main(void) {
    enum InputValue input;

    display_init();
    input_init();

    new_game();

    while(true) {
        display_update_player();
        display_update_bats();

        input = input_next();

        if (game_over()) {
            if (input == BUTTON)
                new_game();
            continue;
        }

        switch (input) {
            case UP: game_move_up(); break;
            case DOWN: game_move_down(); break;
            case LEFT: game_move_left(); break;
            case RIGHT: game_move_right(); break;
            case BUTTON: game_button(); break;
        }

        if (game.state == BAT)
            display_update_bats();

        if (game_over())
            display_draw_map();

        display_update_message();
    }
}
