#include "game.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct Game game;

static void copy_coord(const struct GameCoord* from, struct GameCoord* to) {
    memcpy(to, from, sizeof(struct GameCoord));
}

// the spread and navigation functions are recursive
#pragma static-locals(push, off)

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

#pragma static-locals(pop)

/***
 * @param mask Don't choose tiles with 0 bits set in the mask
 */
static void random_tile(struct GameCoord* coord, uint_fast8_t mask) {
    mask = ~mask;
    do {
        coord->x = rand() % GAME_WIDTH;
        coord->y = rand() % GAME_HEIGHT;
    } while (((game.map[coord->y][coord->x] & mask) & 0xFF) != 0);
}

static void place_wumpus() {
    random_tile(&game.wumpus, MASK_ROOM | MASK_BLOOD | MASK_SLIME);

    game.map[game.wumpus.y][game.wumpus.x] |= MASK_WUMPUS;

    spread(&game.wumpus, MASK_BLOOD, 2);
}

static void place_pits() {
    struct GameCoord* coord;
    for (coord = &(game.pit[0]); coord < &(game.pit[0]) + GAME_PITS; coord++) {
        random_tile(coord, MASK_ROOM | MASK_BLOOD | MASK_SLIME);

        game.map[coord->y][coord->x] |= MASK_PIT;

        spread(coord, MASK_SLIME, 1);
    }
}

static void place_bats() {
    struct GameCoord* coord;
    for (coord = &(game.bats[0]); coord < &(game.bats[0]) + GAME_BATS; coord++) {
        random_tile(coord, MASK_ROOM | MASK_BLOOD | MASK_SLIME);
    }
}

static void place_player() {
    random_tile(&game.player, MASK_ROOM);
}

#pragma static-locals(push, off)

static void count_reachable(struct GameCoord coord, uint_least8_t* count) {
    struct GameCoord newCoord;

    // If we've already visited this room, or this is a pit, stop
    if (game.map[coord.y][coord.x] & (MASK_VISIBLE | MASK_PIT))
        return;

    ++(*count);
    game.map[coord.y][coord.x] |= MASK_VISIBLE;

    newCoord = coord;
    navigate_left(&newCoord);
    count_reachable(newCoord, count);

    newCoord = coord;
    navigate_right(&newCoord);
    count_reachable(newCoord, count);

    newCoord = coord;
    navigate_up(&newCoord);
    count_reachable(newCoord, count);

    newCoord = coord;
    navigate_down(&newCoord);
    count_reachable(newCoord, count);
}

#pragma static-locals(pop)

static bool check_reachable(struct GameCoord* start, uint_least8_t expected) {
    uint_least8_t x, y, count = 0;

    count_reachable(*start, &count);

    for (y = 0; y < GAME_HEIGHT; ++y)
        for (x = 0; x < GAME_WIDTH; ++x)
            game.map[y][x] &= ~MASK_VISIBLE;

    return count == expected - GAME_PITS;
}

void game_new(uint_fast8_t room_count) {
    struct GameCoord coord;
    uint_least8_t i;

    do {
        // Fill the map with random corridors
        for (coord.y = 0; coord.y < GAME_HEIGHT; ++coord.y)
            for (coord.x = 0; coord.x < GAME_WIDTH; ++coord.x)
                game.map[coord.y][coord.x] = (rand() & 1)
                    ? MASK_UL : MASK_UR;

        // Place the rooms
        for (i = 0; i < room_count; ++i) {
            random_tile(&coord, MASK_UL | MASK_UR);
            game.map[coord.y][coord.x] = MASK_ROOM;
        };

        // Place the pits now, as they're considered when determining
        // reachability
        place_pits();

    } while (!check_reachable(&coord, room_count));

    for (coord.x = 0; coord.x < GAME_BATS; coord.x++)
        game.bats_visible[coord.x] = false;

    place_wumpus();
    place_bats();
    place_player();

    game.state = MOVING;
}

uint_fast8_t game_player_tile() {
    return game.map[game.player.y][game.player.x];
}

static bool coords_equal(const struct GameCoord* c1, const struct GameCoord* c2) {
    return c1->x == c2->x && c1->y == c2->y;
}

static void check_player() {
    struct GameCoord* coord;
    uint_least8_t i;

    // The state might be BAT
    game.state = MOVING;

    // Update the visibility of passages
    i = game.map[game.player.y][game.player.x];
    if ((i & MASK_ROOM) == 0) {
        game.map[game.player.y][game.player.x] |=
            game.passage_up
                ? MASK_VISIBLE : MASK_VISIBLE_DOWN;
    }

    for (i = 0; i < GAME_BATS; i++) {
        coord = &game.bats[i];
        if (coords_equal(&game.player, coord)) {
            if (game.bats_visible[i]) {
                // The second visit to a bat
                // Move the player, don't transport to the same place
                do {
                    // re-use the bat coordinates memory for this
                    random_tile(coord, MASK_ROOM | MASK_WUMPUS | MASK_PIT | MASK_BLOOD | MASK_SLIME);
                } while (coords_equal(&game.player, coord));
                copy_coord(coord, &game.player);
                random_tile(coord, MASK_ROOM | MASK_BLOOD | MASK_SLIME);
                game.bats_visible[i] = false;
            } else {
                // The first visit, just show the bat
                game.bats_visible[i] = true;
            }
            game.state = BAT;
        }
    }

    if (coords_equal(&game.player, &game.wumpus)) {
        game.state = WUMPUS;
        return;
    }

    for (coord = &(game.pit[0]); coord < &(game.pit[0]) + GAME_PITS; coord++) {
        if (coords_equal(&game.player, coord)) {
            game.state = PIT;
            return;
        }
    }
}

static void fire(void (*dir_function)(struct GameCoord* coord)) {
    struct GameCoord coord;
    copy_coord(&game.player, &coord);
    dir_function(&coord);
    game.state = coords_equal(&coord, &game.wumpus) ? WON : WUMPUS;
}

static bool is_tile(struct GameCoord* coord, uint_fast8_t mask) {
    return (game.map[coord->y][coord->x] & mask) != 0;
}

bool game_over() {
    return (game.state == WON || game.state == PIT || game.state == WUMPUS);
}

void game_move_up() {
    if (game_over())
        return;

    if (game.state == FIRING) {
        fire(navigate_up);
        return;
    }

    // The only time we can't move is in a passage entered from the bottom
    if (!game.passage_up && is_tile(&game.player, MASK_UL | MASK_UR))
        return;

    game.player.y = (game.player.y == 0) ? GAME_HEIGHT - 1 : game.player.y - 1;
    game.passage_up = false;

    check_player();
}

void game_move_down() {
    if (game_over())
        return;

    if (game.state == FIRING) {
        fire(navigate_down);
        return;
    }

    // The only time we can't move is in a passage entered from the top
    if (game.passage_up && is_tile(&game.player, MASK_UL | MASK_UR))
        return;

    game.player.y = (game.player.y == GAME_HEIGHT - 1) ? 0 : game.player.y + 1;
    game.passage_up = true;

    check_player();
}

void game_move_left() {
    uint_fast8_t tile;

    if (game_over())
        return;

    if (game.state == FIRING) {
        fire(navigate_left);
        return;
    }

    tile = game_player_tile();
    if ((game.passage_up && (tile & MASK_UR)) || (!game.passage_up && (tile & MASK_UL)))
        return;

    game.player.x = (game.player.x == 0) ? GAME_WIDTH - 1 : game.player.x - 1;
    tile = game_player_tile();
    game.passage_up = (tile & MASK_UR) != 0;

    check_player();
}

void game_move_right() {
    uint_fast8_t tile;

    if (game_over())
        return;

    if (game.state == FIRING) {
        fire(navigate_right);
        return;
    }

    tile = game_player_tile();
    if ((game.passage_up && (tile & MASK_UL)) || (!game.passage_up && (tile & MASK_UR)))
        return;

    game.player.x = (game.player.x == GAME_WIDTH - 1) ? 0 : game.player.x + 1;
    tile = game_player_tile();
    game.passage_up = (tile & MASK_UL) != 0;

    check_player();
}

void game_button() {
    if (game.state != MOVING && game.state != BAT)
        return;

    game.state = FIRING;
}