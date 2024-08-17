#include <stdint.h>
#include <stdio.h>
#include <c64.h>

#include "game.h"
#include "display_map.h"
#include "sys.h"

static void init_screen() {
    // Character memory at $3800
    VIC.addr |= 0xE;
}

void main(void) {
    init_screen();

    game_new(128);

    draw_map();
}
