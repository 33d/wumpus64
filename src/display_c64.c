#include "game.h"
#include "sys_c64.h"
#include "display.h"

#include <c64.h>

static const uint8_t TILE_ROOM[] = {
    0, 1, 2, 3, 4, 32, 32, 7, 8, 32, 32, 11, 12, 13, 14, 15
};
static const uint8_t TILE_ULLR[] = {
    16, 17, 18, 32, 19, 20, 21, 22, 23, 24, 25, 26, 32, 27, 28, 29
};
static const uint8_t TILE_URLL[] = {
    32, 30, 31, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 32
};

static void draw_tile_from_indices(const uint8_t* indices, uint8_t* o) {
    uint_fast8_t ty, tx;
    uint_fast8_t index = 0, o_offs = 0;

    for (ty = 0; ty < 4; ++ty) {
        for (tx = 0; tx < 4; tx++)
            o[o_offs++] = indices[index++];
        o_offs += 36;
    }
}

static void draw_tile(uint_fast8_t y, uint_fast8_t x) {
    uint8_t* const tile_start = screenmem + 4 + ((uint16_t) y * 160) + (x * 4);
    
    uint_fast8_t tile_type = game.map[y][x];

    if (tile_type & MASK_ROOM)
        draw_tile_from_indices(TILE_ROOM, tile_start);
    else if (tile_type & MASK_UL)
        draw_tile_from_indices(TILE_ULLR, tile_start);
    else if (tile_type & MASK_UR)
        draw_tile_from_indices(TILE_URLL, tile_start);
}

void display_draw_map() {
    uint_fast8_t y, x;

    for (y = 0; y < GAME_HEIGHT; y++) {
        for (x = 0; x < GAME_WIDTH; x++) {
            draw_tile(y, x);
        }
    }
}

void display_init() {
    // Character memory at $3800
    VIC.addr |= 0xE;
}
