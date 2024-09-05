#include "game.h"
#include "sys_c64.h"
#include "display.h"

#include <string.h>
#include <c64.h>
#include <6502.h>

#define PASSAGE_OFFSET 6

// The messages in order of the GameState enum
static const uint8_t MESSAGES[][2] = {
    { 0, 0 },
    { 0, 0 },
    { 64, 13 },
    { 94, 14 },
    { 77, 16 },
    { 109, 14 }
};

static const uint8_t TILE_ROOM[] = {
    0, 1, 2, 3, 4, 32, 32, 7, 8, 32, 32, 11, 12, 13, 14, 15
};
static const uint8_t TILE_UL[] = {
    16, 17, 18, 32, 19, 20, 46, 32, 23, 47, 32, 32, 32, 32, 32, 32
};
static const uint8_t TILE_UR[] = {
    32, 30, 31, 33, 32, 48, 36, 37, 32, 32, 49, 41, 32, 32, 32, 32
};
static const uint8_t TILE_ULLR[] = {
    16, 17, 18, 32, 19, 20, 21, 22, 23, 24, 25, 26, 32, 27, 28, 29
};
static const uint8_t TILE_LL[] = {
    32, 32, 32, 32, 34, 50, 32, 32, 38, 39, 51, 32, 42, 43, 44, 32
};
static const uint8_t TILE_LR[] = {
    32, 32, 32, 32, 32, 32, 52, 22, 32, 53, 25, 26, 32, 27, 28, 29
};
static const uint8_t TILE_URLL[] = {
    32, 30, 31, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 32
};

// offsets from top left to colour slime cells
static const uint8_t SLIME[] = { 1, 2, 40, 41, 42, 43, 80, 81, 82, 83, 121, 122 };

static void draw_tile_from_indices(
        uint8_t* output, uint8_t* color_output,
        const uint8_t* indices
) {
    uint_fast8_t ty, tx;
    uint_fast8_t index = 0, o_offs = 0;

    for (ty = 0; ty < 4; ++ty) {
        for (tx = 0; tx < 4; tx++) {
            output[o_offs] = indices[index++];
            color_output[o_offs] = COLOR_GRAY1;
            ++o_offs;
        }
        o_offs += 36;
    }
}

static void draw_tiles(
        uint8_t* output, uint8_t* color_output,
        uint_fast8_t output_offset,
        uint_fast8_t start, uint_fast8_t count,
        uint8_t color
) {
    uint8_t i, end = start + count;
    for (i = start; i < end; ++i) {
        output[output_offset] = i;
        color_output[output_offset] = color;
        ++output_offset;
    }
}

static void draw_slime(uint8_t* output) {
    uint8_t i;
    for (i = 0; i < sizeof(SLIME) / sizeof(uint8_t); i++)
        output[SLIME[i]] |= 0x40;
}

static void draw_tile(uint_fast8_t y, uint_fast8_t x) {
    const uint16_t start_offset = 4 + ((uint16_t) y * 160) + (x * 4);
    uint8_t* const tile_start = screenmem + start_offset;
    uint8_t* const color_start = COLOR_RAM + start_offset;
    
    uint_fast8_t tile_type = game.map[y][x];

    if (tile_type & MASK_ROOM)
        draw_tile_from_indices(tile_start, color_start, TILE_ROOM);
    else if (tile_type & MASK_UL) {
        if (game_over() || (tile_type & (MASK_VISIBLE | MASK_VISIBLE_DOWN)) == (MASK_VISIBLE | MASK_VISIBLE_DOWN))
            draw_tile_from_indices(tile_start, color_start, TILE_ULLR);
        else if (tile_type & (MASK_VISIBLE))
            draw_tile_from_indices(tile_start, color_start, TILE_UL);
        else
            draw_tile_from_indices(tile_start, color_start, TILE_LR);
    } else if (tile_type & MASK_UR) {
        if (game_over() || (tile_type & (MASK_VISIBLE | MASK_VISIBLE_DOWN)) == (MASK_VISIBLE | MASK_VISIBLE_DOWN))
            draw_tile_from_indices(tile_start, color_start, TILE_URLL);
        else if (tile_type & (MASK_VISIBLE))
            draw_tile_from_indices(tile_start, color_start, TILE_UR);
        else
            draw_tile_from_indices(tile_start, color_start, TILE_LL);
    }

    // The WUMPUS bit is shared with the corridor VISIBLE_DOWN bit,
    // so check the ROOM bit too
    if ((tile_type & (MASK_ROOM | MASK_WUMPUS)) == (MASK_ROOM | MASK_WUMPUS)) {
        draw_tiles(tile_start, color_start, 41, 54, 2, COLOR_RED);
        draw_tiles(tile_start, color_start, 81, 56, 2, COLOR_RED);
    } else if (tile_type & MASK_BLOOD) {
        draw_tiles(tile_start, color_start, 41, 5, 2, COLOR_RED);
        draw_tiles(tile_start, color_start, 81, 9, 2, COLOR_RED);
    }

    if (tile_type & MASK_SLIME)
        draw_slime(tile_start);

    if (tile_type & MASK_PIT) {
        draw_tiles(tile_start, color_start, 41, 5 + 64, 2, COLOR_BLACK);
        draw_tiles(tile_start, color_start, 81, 9 + 64, 2, COLOR_BLACK);
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

    draw_tile(game.player.y, game.player.x);

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

void display_draw_map() {
    uint_fast8_t y, x;

    for (y = 0; y < GAME_HEIGHT; y++) {
        for (x = 0; x < GAME_WIDTH; x++) {
            draw_tile(y, x);
        }
    }

    for (y = 0; y < GAME_BATS; ++y)
        game.bats_visible[y] = true;
    display_update_bats();
}

void display_update_message() {
    const uint8_t* offsets = MESSAGES[game.state];
    uint8_t* linestart = screenmem + (40 * 24) + 4;
    uint8_t i;
    // fill the line with spaces
    memset(linestart, 32, 32);
    for (i = 0; i < offsets[1]; ++i)
        linestart[i] = i + offsets[0];
}

#pragma optimize(push, off);

void nmi_handler() {
    __asm__("rti");
}

void raster_interrupt_1();
void raster_interrupt_2();

void raster_interrupt_1() {
    __asm__("pha");
    // Delay 38 cycles - that seems to be enough to get the screen mode
    // to change during a border. I can't explain this number - maybe
    // it's a badline, and the interrupt is actually at the start of the
    // previous line.
    __asm__("lda ($0, X)"); // 6
    __asm__("lda ($0, X)"); // 6
    __asm__("lda ($0, X)"); // 6
    __asm__("lda ($0, X)"); // 6
    __asm__("lda ($0, X)"); // 6
    __asm__("lda ($0, X)"); // 6
    __asm__("nop"); // 2
    __asm__("lda #$1b");
    __asm__("sta %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, ctrl1));
    __asm__("lda #251");
    __asm__("sta %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, rasterline));
    // Reset interrupt flag
    __asm__("lda #$FF");
    __asm__("sta %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, irr));
    __asm__("lda #<(%v)", raster_interrupt_2);
    __asm__("sta $FFFE");
    __asm__("lda #>(%v)", raster_interrupt_2);
    __asm__("sta $FFFF");
    __asm__("pla");
    __asm__("rti");
}

void raster_interrupt_2() {
    __asm__("pha");
    __asm__("lda #$5b");
    __asm__("sta %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, ctrl1));
    __asm__("lda #242");
    __asm__("sta %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, rasterline));
    // Reset interrupt flag
    __asm__("lda #$FF");
    __asm__("sta %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, irr));
    __asm__("lda #<(%v)", raster_interrupt_1);
    __asm__("sta $FFFE");
    __asm__("lda #>(%v)", raster_interrupt_1);
    __asm__("sta $FFFF");
    __asm__("pla");
    __asm__("rti");
}

void init_raster_interrupt() {
    __asm__("sei");

    // Kernal ROM off
    __asm__("lda #%b", ~0x07);
    __asm__("and 1");
    __asm__("ora #$05");
    __asm__("sta 1");

    // Disable CIA interrupts
    __asm__("lda #$7F");
    __asm__("sta %w+%b", (uint16_t) &CIA1, offsetof(struct __6526, icr));
    __asm__("sta %w+%b", (uint16_t) &CIA2, offsetof(struct __6526, icr));

    // Clear VIC interrupt flag
    __asm__("and %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, irr));
    __asm__("sta %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, irr));

    // Acknowledge CIA interrupts
    __asm__("sta %w+%b", (uint16_t) &CIA1, offsetof(struct __6526, icr));
    __asm__("sta %w+%b", (uint16_t) &CIA2, offsetof(struct __6526, icr));

    // Set scanline
    __asm__("lda #242");
    __asm__("sta %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, rasterline));

    // Stop the RESTORE key crashing the program
    __asm__("lda #<(%v)", nmi_handler);
    __asm__("sta $FFFA");
    __asm__("lda #>(%v)", nmi_handler);
    __asm__("sta $FFFB");

    // Set interrupt routine
    __asm__("lda #<(%v)", raster_interrupt_1);
    __asm__("sta $FFFE");
    __asm__("lda #>(%v)", raster_interrupt_1);
    __asm__("sta $FFFF");

    // Raster interrupts on
    __asm__("lda #1");
    __asm__("sta %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, imr));

    __asm__("cli");
}

#pragma optimize(pop);

typedef void (*void_func) (void);

void display_init() {
    uint8_t* ptr = screenmem;

    // Extended background mode, and set raster interrupt high bit
    VIC.ctrl1 = 0x5b;
    VIC.ctrl2 = 0x08;

    // Set character memory location
    VIC.addr &= ~0xE;
    VIC.addr |= charmem_reg;

    init_raster_interrupt();

    // Set colours
    VIC.bordercolor = COLOR_GRAY1;
    // Default background colour
    VIC.bgcolor0 = COLOR_GRAY3;
    // The slime colour
    VIC.bgcolor1 = COLOR_GREEN;
    // The space down the sides
    VIC.bgcolor3 = COLOR_GRAY1;

    memset(screenmem, 32, 25 * 40);

    // Fill in the sides of the screen
    memset(ptr, 32 + 192, 4);
    for (ptr += 36; ptr < screenmem + 996 - 40; ptr += 40)
        memset(ptr, 32 + 192, 8);
    memset(ptr, 32 + 192, 4);
    // The last line uses standard character mode
    memset(screenmem + 40 * 24, 255, 4);
    memset(screenmem + 40 * 24 + 36, 255, 4);
    // Fill the rest with spaces
    memset(screenmem + 40 * 24 + 4, 32, 4);
    // Set the colours for the last line
    memset(COLOR_RAM + 40 * 24, COLOR_GRAY1, 40);

    // Set the sprite pointers
    *(screenmem + 0x3f8) = spritedata_reg;
    *(screenmem + 0x3f9) = spritedata_reg + 1;
    *(screenmem + 0x3fa) = spritedata_reg + 2;
    *(screenmem + 0x3fb) = spritedata_reg + 3;
    *(screenmem + 0x3fc) = spritedata_reg + 4;
    *(screenmem + 0x3fd) = spritedata_reg + 5;

    // Sprite colors for player
    VIC.spr0_color = COLOR_YELLOW;
    VIC.spr1_color = COLOR_BLACK;

    // Sprite colors for bats
    VIC.spr2_color = VIC.spr4_color = COLOR_GRAY3;
    VIC.spr3_color = VIC.spr5_color = COLOR_BLACK;
    
    // Player sprite on
    VIC.spr_ena = 0x3;
}

void display_end() {
    __asm__("sei");

    // Sprites off
    __asm__("lda #0");
    __asm__("sta %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, spr_ena));

    // Raster interrupts off
    __asm__("lda #0");
    __asm__("sta %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, imr));
    // Reset interrupt flag
    __asm__("lda #$FF");
    __asm__("sta %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, irr));

    // Kernal ROM on
    __asm__("lda #%b", ~0x07);
    __asm__("and 1");
    __asm__("ora #6");
    __asm__("sta 1");

    __asm__("cli");
}
