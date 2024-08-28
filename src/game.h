#if !defined(GAME_H)
#define GAME_H

#include <stdint.h>
#include <stdbool.h>

#define GAME_WIDTH 8
#define GAME_HEIGHT 6
#define GAME_BATS 2
#define GAME_PITS 1

#define MASK_ROOM 1
#define MASK_UL 2
#define MASK_UR 4
#define MASK_WUMPUS 8
#define MASK_BLOOD 16
#define MASK_PIT 32
#define MASK_SLIME 64
#define MASK_VISIBLE_DOWN 8
#define MASK_VISIBLE 128

enum GameState {
    MOVING, BAT, FIRING, WON, WUMPUS, PIT
};

struct GameCoord {
    uint_fast8_t x, y;
};

struct Game {
    uint_fast8_t map[GAME_HEIGHT][GAME_WIDTH];
    enum GameState state;
    struct GameCoord player;
    // If the player is in passage, are they in the one that leads up?
    bool passage_up;
    struct GameCoord wumpus;
    struct GameCoord pit[GAME_PITS];
    struct GameCoord bats[GAME_BATS];
    bool bats_visible[GAME_BATS];
};

extern struct Game game;

void game_new(uint_fast8_t room_chance);
uint_fast8_t game_player_tile();

void game_move_up();
void game_move_down();
void game_move_left();
void game_move_right();
void game_button();
bool game_over();

#endif
