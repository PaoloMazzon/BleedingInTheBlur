#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
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

// From the tileset
static int32_t get_floor_tile() {
    return random_int(17 + 8, 22 + 8);
}

// These only apply to the base tileset
static bool is_floor_tile(int32_t tile) { return tile >= (17 + 8) && tile <= (21 + 8); }
static bool is_wall_tile(int32_t tile) { return !is_floor_tile(tile); }

static int32_t get_wall_tile() {
    return random_int(1, 17);
}

static int32_t sign(int32_t x) {
    return x > 0 ? 1 : (x < 0 ? -1 : 0);
}

// Applies shadows to decorations
static void autotile(Oct_Tilemap tilemap, Oct_Tilemap decoration, int32_t x, int32_t y) {
    // These are true if there is empty space in that location
    const bool above        = is_floor_tile(oct_GetTilemap(tilemap, x, y - 1));
    const bool below        = is_floor_tile(oct_GetTilemap(tilemap, x, y + 1));
    const bool left         = is_floor_tile(oct_GetTilemap(tilemap, x - 1, y));
    const bool right        = is_floor_tile(oct_GetTilemap(tilemap, x + 1, y));
    const bool top_left     = is_floor_tile(oct_GetTilemap(tilemap, x - 1, y - 1));
    const bool top_right    = is_floor_tile(oct_GetTilemap(tilemap, x + 1, y - 1));
    const bool bottom_left  = is_floor_tile(oct_GetTilemap(tilemap, x - 1, y + 1));
    const bool bottom_right = is_floor_tile(oct_GetTilemap(tilemap, x + 1, y + 1));

    const bool is_wall = !is_floor_tile(oct_GetTilemap(tilemap, x, y));

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

// Find spots for each room and carve them out
// TODO - All empty spaces should be surrounded by special walls that tell the pathfinding
//        algorithm that those spots aren't allowed to be carved out as to prevent hallways
//        from overlapping/running side by side/carving chunks out of rooms
void room_placement_pass(LevelGeneratingState *state) {
    // TODO: This
}

// Uses A* to carve paths between some rooms
// TODO - All empty spaces should be surrounded by special walls that tell the pathfinding
//        algorithm that those spots aren't allowed to be carved out as to prevent hallways
//        from overlapping/running side by side/carving chunks out of rooms
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
    room_placement_pass(&state);
    hallway_placement_pass(&state);
    spawn_locating_pass(&state);
    aesthetics_pass(&state);
    place_stairs_pass(&state);

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
