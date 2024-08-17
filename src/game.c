#include "game.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct Game game;

static void copy_coord(const struct GameCoord* from, struct GameCoord* to) {
    memcpy(to, from, sizeof(struct GameCoord));
}

static void navigate_up(struct GameCoord* coord);
static void navigate_down(struct GameCoord* coord);
static void navigate_left(struct GameCoord* coord);
static void navigate_right(struct GameCoord* coord);

static void navigate_up(struct GameCoord* coord) {
    uint_fast8_t tile;
    coord->y = coord->y == 0 ? GAME_HEIGHT - 1 : coord->y - 1;
    tile = game.map[coord->y][coord->x];
    if (tile & MASK_UL)
        navigate_right(coord);
    else if (tile & MASK_UR)
        navigate_left(coord);
}

static void navigate_down(struct GameCoord* coord) {
    uint_fast8_t tile;
    coord->y = coord->y == GAME_HEIGHT - 1 ? 0 : coord->y + 1;
    tile = game.map[coord->y][coord->x];
    if (tile & MASK_UL)
        navigate_left(coord);
    else if (tile & MASK_UR)
        navigate_right(coord);
}

static void navigate_right(struct GameCoord* coord) {
    uint_fast8_t tile;
    coord->x = coord->x == GAME_WIDTH - 1 ? 0 : coord->x + 1;
    tile = game.map[coord->y][coord->x];
    if (tile & MASK_UL)
        navigate_up(coord);
    else if (tile & MASK_UR)
        navigate_down(coord);
}

static void navigate_left(struct GameCoord* coord) {
    uint_fast8_t tile;
    coord->x = coord->x == 0 ? GAME_WIDTH - 1 : coord->x - 1;
    tile = game.map[coord->y][coord->x];
    if (tile & MASK_UL)
        navigate_down(coord);
    else if (tile & MASK_UR)
        navigate_up(coord);
}

static void spread(const struct GameCoord *coord, 
        uint_fast8_t mask, uint_fast8_t iterations) {
    struct GameCoord next;
    game.map[coord->y][coord->x] |= mask;
    if (iterations == 0)
        return;

    --iterations;

    copy_coord(coord, &next);
    navigate_up(&next);
    spread(&next, mask, iterations);

    copy_coord(coord, &next);
    navigate_down(&next);
    spread(&next, mask, iterations);

    copy_coord(coord, &next);
    navigate_left(&next);
    spread(&next, mask, iterations);

    copy_coord(coord, &next);
    navigate_right(&next);
    spread(&next, mask, iterations);
}

static void random_room(struct GameCoord* coord, uint_fast8_t mask) {
    mask = ~mask;
    do {
        coord->x = rand() % GAME_WIDTH;
        coord->y = rand() % GAME_HEIGHT;
    } while (((game.map[coord->y][coord->x] & mask) & 0xFF) != 0);
}

static void place_wumpus() {
    random_room(&game.wumpus, MASK_ROOM | MASK_BLOOD | MASK_SLIME);

    game.map[game.wumpus.y][game.wumpus.x] |= MASK_WUMPUS;

    spread(&game.wumpus, MASK_BLOOD, 2);
}

static void place_pits() {
    struct GameCoord* coord;
    for (coord = &(game.pit[0]); coord < &(game.pit[0]) + GAME_PITS; coord++) {
        random_room(coord, MASK_ROOM | MASK_BLOOD | MASK_SLIME);

        game.map[coord->y][coord->x] |= MASK_PIT;

        spread(coord, MASK_SLIME, 1);
    }
}

static void place_bats() {
    struct GameCoord* coord;
    for (coord = &(game.bats[0]); coord < &(game.bats[0]) + GAME_BATS; coord++) {
        random_room(coord, MASK_ROOM | MASK_BLOOD | MASK_SLIME);
    }
}

static void place_player() {
    random_room(&game.player, MASK_ROOM);
}

void game_new(uint_fast8_t room_chance) {
    uint_fast8_t x, y;
    for (y = 0; y < GAME_HEIGHT; y++)
        for (x = 0; x < GAME_WIDTH; x++) {
            if ((rand() & 0xFF) < room_chance)
                game.map[y][x] = MASK_ROOM;
            else if ((rand() & 0xFF) < 128)
                game.map[y][x] = MASK_UL;
            else
                game.map[y][x] = MASK_UR;
        }

    place_wumpus();
    place_pits();
    place_bats();
    place_player();
}

bool coords_equal(const struct GameCoord* c1, const struct GameCoord* c2) {
    return c1->x == c2->x && c1->y == c2->y;
}

enum MoveResult check_player() {
    struct GameCoord* coord;

    if (coords_equal(&game.player, &game.wumpus))
        return WUMPUS;

    for (coord = &(game.pit[0]); coord < &(game.pit[0]) + GAME_PITS; coord++) {
        if (coords_equal(&game.player, coord))
            return PIT;
    }

    for (coord = &(game.bats[0]); coord < &(game.bats[0]) + GAME_BATS; coord++) {
        if (coords_equal(&game.player, coord)) {
            random_room(&game.player, MASK_ROOM | MASK_WUMPUS | MASK_PIT);
            return MOVED;
        }
    }

    return MOVED;
}

bool is_tile(struct GameCoord* coord, uint_fast8_t mask) {
    return (game.map[coord->y][coord->x] & mask) != 0;
}

enum MoveResult game_move_up() {
    // The only time we can't move is in a passage entered from the bottom
    if (!game.passage_up && is_tile(&game.player, MASK_UL | MASK_UR))
        return NOTHING;

    game.player.y = (game.player.y == 0) ? GAME_HEIGHT - 1 : game.player.y - 1;
    game.passage_up = false;

    return check_player();
}

enum MoveResult game_move_down() {
    // The only time we can't move is in a passage entered from the top
    if (game.passage_up && is_tile(&game.player, MASK_UL | MASK_UR))
        return NOTHING;

    game.player.y = (game.player.y == GAME_HEIGHT - 1) ? 0 : game.player.y + 1;
    game.passage_up = true;

    return check_player();
}

enum MoveResult game_move_left() {
    uint_fast8_t tile = game.map[game.player.y][game.player.x];
    if (!game.passage_up && tile == MASK_UL || game.passage_up && tile == MASK_UR)
        return NOTHING;

    game.player.y = (game.player.y == 0) ? GAME_HEIGHT - 1 : game.player.y - 1;
    game.passage_up = false;

    return check_player();
}

enum MoveResult game_move_right() {
    uint_fast8_t tile = game.map[game.player.y][game.player.x];
    if (game.passage_up && tile == MASK_UL || !game.passage_up && tile == MASK_UR)
        return NOTHING;

    game.player.x = (game.player.x == GAME_WIDTH - 1) ? 0 : game.player.x + 1;

    return check_player();
}
