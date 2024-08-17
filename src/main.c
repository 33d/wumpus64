#include <stdint.h>
#include <stdio.h>

#include "game.h"
#include "display.h"

void main(void) {
    display_init();

    game_new(128);

    display_draw_map();
}
