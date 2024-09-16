#include <stdint.h>
#include <stdlib.h>

#include "sys.h"
#include "game.h"
#include "display.h"
#include "input.h"
#include "mainmenu.h"
#include "end.h"

static uint8_t ROOMS[] = { 32, 24, 18 };

static enum EndMenuState play_level(enum Difficulty difficulty) {
    enum InputValue input;
    enum EndMenuState end_state;

    game_new(ROOMS[difficulty]);

    display_init();

    while(true) {
        display_update_player();
        display_update_bats();

        if (game_over()) {
            end_state = get_end_state();
            display_end();
            return end_state;
        }

        do {
            input = input_next();
        } while (input == NONE || input == CENTER);

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

void main(void) {
    enum Difficulty difficulty;
    enum EndMenuState end_state;

    sys_init();
    input_init();

    while (1) {
        difficulty = main_menu_get_selection();

        srand(sys_random_seed());

        do {
            end_state = play_level(difficulty);
        } while (end_state == AGAIN);
    }
}
