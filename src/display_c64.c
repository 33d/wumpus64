#include "game.h"
#include "sys_c64.h"
#include "display.h"

#include <c64.h>

#define PASSAGE_OFFSET 6

static const uint8_t TILE_ROOM[] = {
    0, 1, 2, 3, 4, 32, 32, 7, 8, 32, 32, 11, 12, 13, 14, 15
};
static const uint8_t TILE_ULLR[] = {
    16, 17, 18, 32, 19, 20, 21, 22, 23, 24, 25, 26, 32, 27, 28, 29
};
static const uint8_t TILE_URLL[] = {
    32, 30, 31, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 32
};

static void draw_tile_from_indices(uint8_t* output, const uint8_t* indices) {
    uint_fast8_t ty, tx;
    uint_fast8_t index = 0, o_offs = 0;

    for (ty = 0; ty < 4; ++ty) {
        for (tx = 0; tx < 4; tx++)
            output[o_offs++] = indices[index++];
        o_offs += 36;
    }
}

static void draw_tiles(uint8_t* output, uint_fast8_t output_offset, uint_fast8_t start, uint_fast8_t count) {
    uint8_t i, end = start + count;
    for (i = start; i < end; ++i) {
        output[output_offset++] = i;
    }
}

static void draw_tile(uint_fast8_t y, uint_fast8_t x) {
    uint8_t* const tile_start = screenmem + 4 + ((uint16_t) y * 160) + (x * 4);
    
    uint_fast8_t tile_type = game.map[y][x];

    if (tile_type & MASK_ROOM)
        draw_tile_from_indices(tile_start, TILE_ROOM);
    else if (tile_type & MASK_UL)
        draw_tile_from_indices(tile_start, TILE_ULLR);
    else if (tile_type & MASK_UR)
        draw_tile_from_indices(tile_start, TILE_URLL);

    if (tile_type & MASK_WUMPUS) {
        draw_tiles(tile_start, 41, 54, 2);
        draw_tiles(tile_start, 81, 56, 2);
    } else if (tile_type & MASK_BLOOD) {
        draw_tiles(tile_start, 41, 5, 2);
        draw_tiles(tile_start, 81, 9, 2);
    }
}

void display_draw_map() {
    uint_fast8_t y, x;

    for (y = 0; y < GAME_HEIGHT; y++) {
        for (x = 0; x < GAME_WIDTH; x++) {
            draw_tile(y, x);
        }
    }
}

static void draw_sprite(
    uint_least16_t sprite,
    struct GameCoord* coords,
    int8_t xoff, int8_t yoff
) {
    uint_fast16_t x;
    struct { unsigned char x; unsigned char y; }
        *fgcoords = &VIC.spr_pos[sprite],
        *bgcoords = &VIC.spr_pos[sprite + 1];

    x = 24    // ???
        + 32  // map is offset by 4 tiles
        + 4   // sprite is centred in its 24 pixels
        + (uint_fast16_t) coords->x * 32
        + xoff;
    fgcoords->x = bgcoords->x = x;
    // Set the 9th bit of the x coordinate
    if (x > 255)
        VIC.spr_hi_x |= (3 << sprite);
    else
        VIC.spr_hi_x &= ~(3 << sprite);

    fgcoords->y = bgcoords->y =
        50  // ???
        + 8 // sprite is in the top 16 pixels
        + coords->y * 32
        + yoff;
}

void display_update_player() {
    int8_t xoff = 0, yoff = 0;
    uint_fast8_t tile = game_player_tile();

    // If the player is in a passage, move to one side
    if (tile & MASK_UL) {
        if (game.passage_up) {
            xoff = -PASSAGE_OFFSET;
            yoff = -PASSAGE_OFFSET;
        } else {
            xoff =  PASSAGE_OFFSET;
            yoff =  PASSAGE_OFFSET;
        }
    } else if (tile & MASK_UR) {
        if (game.passage_up) {
            xoff =  PASSAGE_OFFSET;
            yoff = -PASSAGE_OFFSET;
        } else {
            xoff = -PASSAGE_OFFSET;
            yoff =  PASSAGE_OFFSET;
        }
    }

    draw_sprite(0, &game.player, xoff, yoff);
}

void display_update_bats() {
    uint_least8_t i;
    for (i = 0; i < GAME_BATS; i++) {
        if (game.bats_visible[i])
            VIC.spr_ena |= 3 << ((i + 1) * 2);
        else {
            draw_sprite((i + 1) * 2, &game.bats[i], 0, 0);
            VIC.spr_ena &= ~(3 << ((i + 1) * 2));
        }
    }
}

void display_init() {
    const uint8_t sprite_ptr = ((const uint16_t) spritedata / 64);

    // Character memory at $3800
    VIC.addr |= 0xE;

    // Set the sprite pointers
    *(screenmem + 0x3f8) = sprite_ptr;
    *(screenmem + 0x3f9) = sprite_ptr + 1;
    *(screenmem + 0x3fa) = sprite_ptr + 2;
    *(screenmem + 0x3fb) = sprite_ptr + 3;
    *(screenmem + 0x3fc) = sprite_ptr + 4;
    *(screenmem + 0x3fd) = sprite_ptr + 5;

    // Sprite colors for player
    VIC.spr0_color = COLOR_YELLOW;
    VIC.spr1_color = COLOR_BLACK;

    // Sprite colors for bats
    VIC.spr2_color = VIC.spr4_color = COLOR_GRAY3;
    VIC.spr3_color = VIC.spr5_color = COLOR_BLACK;
    
    // Player sprite on
    VIC.spr_ena = 0x3;
}
