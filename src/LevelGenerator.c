#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "LevelGenerator.h"
#include "Util.h"
#include "Game.h"

static int32_t current_prop_index;

typedef enum {
    PROP_LOCATION_FLOOR = 0,
    PROP_LOCATION_AGAINST_WALL = 1,
    PROP_LOCATION_ON_WALL = 2,
    PROP_LOCATION_MAX = 3,
} PropLocation;

// For keeping tracks of room in the dungeon while level generating
typedef struct RoomSpace_s {
    Position top_left;
    IntRange size;
} RoomSpace;

typedef struct LevelGeneratingState_s {
    RoomSpace *rooms;
    int32_t room_count;
    Oct_Tilemap base_tilemap;
    Oct_Tilemap shading_tilemap;
    LevelGenerationParameters params;
} LevelGeneratingState;

static void debug_print_level(LevelGeneratingState *state) {
#ifdef NDEBUG
    return;
#endif
    for (int32_t y = 0; y < state->params.level_size[1]; y++) {
        for (int32_t x = 0; x < state->params.level_size[0]; x++) {
            TileContents *t = level_get_tile((Position){x, y});
            if (t->type == TILE_CONTENTS_TYPE_WALL && t->tile.room_edge)
                printf("~~");
            else if (t->type == TILE_CONTENTS_TYPE_WALL && !t->tile.room_edge)
                printf("##");
            else if (t->type == TILE_CONTENTS_TYPE_NONE)
                printf("..");
        }
        printf("\n");
    }
    fflush(stdout);
}

// Returns a random floor tile from the tileset
static int32_t get_floor_tile() {
    return random_int(17 + 8, 22 + 8);
}

// Returns a random wall tile from the tileset
static int32_t get_wall_tile() {
    return random_int(1, 17);
}

static int32_t sign(int32_t x) {
    return x > 0 ? 1 : (x < 0 ? -1 : 0);
}

// Returns true if its a wall on the perimeter of a room or hallway
static bool is_edge_tile(Position pos) {
    TileContents *t = level_get_tile(pos);
    return t && t->type == TILE_CONTENTS_TYPE_WALL && t->tile.room_edge;
}

// Returns true if the tile is a wall, doesn't care if its an edge or not
static bool is_wall_tile(Position pos) {
    TileContents *t = level_get_tile(pos);
    return t && t->type == TILE_CONTENTS_TYPE_WALL;
}

// Does the work involved in setting a tile to a wall, if its on the perimeter of a hallway or room set edge_tile to true
static void set_wall_tile(LevelGeneratingState *state, Position pos, bool edge_tile) {
    TileContents *t = level_get_tile(pos);
    if (t) {
        t->type = TILE_CONTENTS_TYPE_WALL;
        t->tile.room_edge = edge_tile;
        oct_SetTilemap(state->base_tilemap, pos[0], pos[1], get_wall_tile());
    }
}

// Same as set_wall_tile but for floors
static void set_floor_tile(LevelGeneratingState *state, Position pos) {
    TileContents *t = level_get_tile(pos);
    if (t) {
        t->type = TILE_CONTENTS_TYPE_NONE;
        oct_SetTilemap(state->base_tilemap, pos[0], pos[1], get_floor_tile());
    }
}

static bool is_floor_tile(Position pos) {
    return !is_wall_tile(pos);
}

// Returns true if any part of the potential room location is occupied by wall perimeters
static bool is_space_occupied(RoomSpace *potential_room_location) {
    for (int32_t y = potential_room_location->top_left[1]; y < potential_room_location->top_left[1] + potential_room_location->size[1]; y++) {
        for (int32_t x = potential_room_location->top_left[0]; x < potential_room_location->top_left[0] + potential_room_location->size[0]; x++) {
            if (is_edge_tile((Position){x, y}) || is_floor_tile((Position){x, y}))
                return true;
        }
    }
    return false;
}

// The outer perimeter will be made edge tiles
static void carve_out_space_for_room(LevelGeneratingState *state, RoomSpace *room) {
    for (int32_t y = room->top_left[1]; y < room->top_left[1] + room->size[1]; y++) {
        for (int32_t x = room->top_left[0]; x < room->top_left[0] + room->size[0]; x++) {
            if (y == room->top_left[1] || y == (room->top_left[1] + room->size[1] - 1) ||
                x == room->top_left[0] || x == (room->top_left[0] + room->size[0] - 1))
                set_wall_tile(state, (Position){x, y}, true);
            else
                set_floor_tile(state, (Position){x, y});
        }
    }
}

// Applies shadows to decorations
static void autotile(Oct_Tilemap tilemap, Oct_Tilemap decoration, int32_t x, int32_t y) {
    // These are true if there is empty space in that location
    const bool above        = is_floor_tile((Position){x, y - 1});
    const bool below        = is_floor_tile((Position){x, y + 1});
    const bool left         = is_floor_tile((Position){x - 1, y});
    const bool right        = is_floor_tile((Position){x + 1, y});
    const bool top_left     = is_floor_tile((Position){x - 1, y - 1});
    const bool top_right    = is_floor_tile((Position){x + 1, y - 1});
    const bool bottom_left  = is_floor_tile((Position){x - 1, y + 1});
    const bool bottom_right = is_floor_tile((Position){x + 1, y + 1});

    const bool is_wall = !is_wall_tile((Position) {x, y});

    if (is_wall) {
        const int open = (above ? 1 : 0) + (below ? 1 : 0) + (left ? 1 : 0) + (right ? 1 : 0);

        if (open == 0) {
            if      (top_right)    oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_INNER_CORNER_TR);
            else if (top_left)     oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_INNER_CORNER_TL);
            else if (bottom_right) oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_INNER_CORNER_BR);
            else if (bottom_left)  oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_INNER_CORNER_BL);

        } else if (open == 1) {
            if      (above) oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_ABOVE);
            else if (below) oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_BELOW);
            else if (left)  oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_LEFT);
            else if (right) oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_RIGHT);

        } else if (open == 2) {
            if      (above && below)  oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_ABOVE_BELOW);
            else if (left  && right)  oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_LEFT_RIGHT);
            else if (above && right)  oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_OUTER_CORNER_TR);
            else if (above && left)   oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_OUTER_CORNER_TL);
            else if (below && right)  oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_OUTER_CORNER_BR);
            else if (below && left)   oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_OUTER_CORNER_BL);

        } else if (open == 3) {
            if      (!left)  oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_ABOVE_BELOW_RIGHT);
            else if (!right) oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_ABOVE_BELOW_LEFT);
            else if (!below) oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_LEFT_RIGHT_TOP);
            else if (!above) oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_LEFT_RIGHT_BOTTOM);
        }
    } else {
        const bool wall_right = !right;
        const bool wall_above = !above;
        const bool wall_top_right = !top_right;

        if      (wall_right && wall_above) oct_SetTilemap(decoration, x, y, TILE_FLOOR_SHADOW_CORNER);
        else if (wall_right)               oct_SetTilemap(decoration, x, y, TILE_FLOOR_SHADOW_LEFT);
        else if (wall_above)               oct_SetTilemap(decoration, x, y, TILE_FLOOR_SHADOW_BELOW);
        else if (wall_top_right)           oct_SetTilemap(decoration, x, y, TILE_WALL_SHADOW_INNER_CORNER_TR);
    }
}

// Fills the entire room with walls
static void fill_pass(LevelGeneratingState *state) {
    for (int32_t y = 0; y < state->params.level_size[1]; y++) {
        for (int32_t x = 0; x < state->params.level_size[0]; x++) {
            TileContents *t = level_get_tile((Position){x, y});
            assert(t);
            t->type = TILE_CONTENTS_TYPE_WALL;
            oct_SetTilemap(state->base_tilemap, x, y, get_wall_tile());
        }
    }
}

// Find spots for each room and carve them out
void room_placement_pass(LevelGeneratingState *state) {
    /// 1. For each room,
    ///   a) Pick a random room size within the specified range
    ///   b) Pick a random location in the level to place it
    ///   c) If that location is already occupied, try again up to a total of 10 times
    ///   d) If there is no location found on the 10th try, pick the smallest possible room size and try another 10 times
    ///   e) If that still doesn't work, stop placing rooms
    /// 3. Done
    int32_t room_pointer = 0;
    debug("Starting room pass with count %i.", state->room_count);

    const IntRange min_room_size = {
            state->params.room_min_size[0],
            state->params.room_min_size[1]
    };
    bool revert_to_lowest_size = false;
    for (int32_t i = 0; i < state->room_count; i++) {
        IntRange room_size = {
                random_int(state->params.room_min_size[0], state->params.room_max_size[0] + 1),
                random_int(state->params.room_min_size[1], state->params.room_max_size[1] + 1),
        };
        if (revert_to_lowest_size) {
            room_size[0] = min_room_size[0];
            room_size[1] = min_room_size[1];
        }
        int32_t attempts_left = 10;
        bool found_spot_for_a_room = false;
        while (attempts_left > 0 && !found_spot_for_a_room) {
            IntRange top_left = {
                    random_int(2, state->params.level_size[0] - room_size[0] - 4),
                    random_int(2, state->params.level_size[1] - room_size[1] - 4),
            };
            RoomSpace room = {
                    .top_left = {top_left[0] - 1, top_left[1] - 1},
                    .size = {room_size[0] + 2, room_size[1] + 2}
            };
            if (!is_space_occupied(&room)) {
                found_spot_for_a_room = true;
                carve_out_space_for_room(state, &room);
                memcpy(&state->rooms[room_pointer++], &room, sizeof(RoomSpace));
                debug("Found room location.");
            } else {
                attempts_left -= 1;
            }
        }

        if (!found_spot_for_a_room && !revert_to_lowest_size) {
            revert_to_lowest_size = true;
            state->room_count += 1;
            debug("Ran out of space for room size [%i,%i] at room %i. Reverting to smallest room.", room_size[0], room_size[1], i);
        } else if (!found_spot_for_a_room /* && revert_to_lowest_size */) {
            debug("Ran out of space for more rooms at room %i", i);
            break;
        }
    }
    state->room_count = room_pointer;
    debug_print_level(state);
}

// Uses A* to carve paths between some rooms
void hallway_placement_pass(LevelGeneratingState *state) {
    // TODO: This
}

// Finds a large amount of possible spawn points for things like items and characters
void spawn_locating_pass(LevelGeneratingState *state) {
    // TODO: This
}

// Does the shading auto-tiling, places doors, places props
void aesthetics_pass(LevelGeneratingState *state) {
    for (int32_t y = 0; y < state->params.level_size[1]; y++) {
        for (int32_t x = 0; x < state->params.level_size[0]; x++) {
            autotile(state->base_tilemap, state->shading_tilemap, x, y);
            // TODO: Place doors
        }
    }
}

// Places the stairs up and down
void place_stairs_pass(LevelGeneratingState *state) {
    // TODO: This
}

void generate_level(Level *level, LevelGenerationParameters *params, Position out_player_pos) {
    const int32_t spawns_per_room = 5;
    current_prop_index = 0;

    // Level generation needs at least 3 rooms
    assert(params->room_count[0] > 2 && params->room_count[0] > 2);

    // Max room size must be less than the level size
    assert(params->room_max_size[0] < params->level_size[0]);
    assert(params->room_max_size[1] < params->level_size[1]);

    // Rooms must have at least spawns_per_room spawnable spots
    assert(params->room_min_size[0] * params->room_max_size[1] >= spawns_per_room);

    // Create resources and pick a room count
    TileContents *level_tiles = oct_Zalloc(
        g_game.allocator,
        sizeof(TileContents) * params->level_size[0] * params->level_size[1]);
    LevelGeneratingState state;
    state.base_tilemap = oct_CreateTilemap(
            oct_GetAsset(g_game.assets, "tileset.png"),
            params->level_size[0], params->level_size[1],
            (Oct_Vec2){CELL_WIDTH, CELL_HEIGHT});
    state.shading_tilemap = oct_CreateTilemap(
            oct_GetAsset(g_game.assets, "tileset.png"),
            params->level_size[0], params->level_size[1],
            (Oct_Vec2){CELL_WIDTH, CELL_HEIGHT});
    state.room_count = random_int(params->room_count[0], params->room_count[1] + 1);
    state.rooms = oct_Malloc(g_game.allocator, sizeof(RoomSpace) * state.room_count);
    level->tiles = level_tiles;
    level->tilemap = state.base_tilemap;
    level->decorations = state.shading_tilemap;
    level->level_width = params->level_size[0];
    level->level_height = params->level_size[1];
    state.params = *params;

    // Do each pass required to generate the level
    Position stairs_down, stairs_up;
    fill_pass(&state);
    room_placement_pass(&state);
    hallway_placement_pass(&state);
    spawn_locating_pass(&state);
    aesthetics_pass(&state);
    place_stairs_pass(&state);
    debug_print_level(&state);

    // Draw the entire level to a texture
    level->level_tex = oct_CreateSurface((Oct_Vec2){(float)level->level_width * CELL_WIDTH, (float)level->level_height * CELL_HEIGHT});
    assert(level->level_tex);

    // Place the player
    out_player_pos[0] = stairs_down[0];
    out_player_pos[1] = stairs_down[1];
    oct_Free(g_game.allocator, state.rooms);
}

void cleanup_level(Level *level) {
    oct_Free(g_game.allocator, level->spawn_points);
    level->spawn_points = nullptr;
    oct_FreeAsset(level->level_tex);
    level->level_tex = OCT_NO_ASSET;
    oct_Free(g_game.allocator, level->tiles);
    level->tiles = nullptr;
    oct_DestroyTilemap(level->tilemap);
    level->tilemap = nullptr;
    oct_DestroyTilemap(level->decorations);
    level->tilemap = nullptr;
}
