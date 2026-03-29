#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "LevelGenerator.h"
#include "Util.h"
#include "Game.h"



// For keeping tracks of room in the dungeon while level generating
typedef struct RoomSpace_s {
    Position top_left;
    IntRange size;
} RoomSpace;

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

// Returns true if two rooms are overlapping or their borders are touching
static bool rooms_overlapping(RoomSpace *r1, RoomSpace *r2) {
    if (r1->top_left[0] - 1 <= r2->top_left[0] + r2->size[0] + 2 &&
        r1->top_left[1] - 1 <= r2->top_left[1] + r2->size[1] + 2 &&
        r1->top_left[0] + r1->size[0] + 2 >= r2->top_left[0] - 1 &&
        r1->top_left[1] + r1->size[1] + 2 >= r2->top_left[1] - 1)
        return true;
    return false;
}

// Returns true if a tile exists in a room
static bool tile_in_room(RoomSpace *room_list, int32_t room_count, Position p) {
    for (int32_t i = 0; i < room_count; i++) {
        if (p[0] >= room_list[i].top_left[0] &&
            p[0] <= room_list[i].top_left[0] + room_list[i].size[0] &&
            p[1] >= room_list[i].top_left[1] &&
            p[1] <= room_list[i].top_left[1] + room_list[i].size[1])
            return true;
    }
    return false;
}

// Returns false if it fails to find a spot for that room in 10 attempts
static bool find_space_for_room(RoomSpace *rooms, int32_t room_count, IntRange level_size, IntRange room_size, Position out_top_left) {
    for (int32_t i = 0; i < 10; i++) {
        RoomSpace r1 = {
            .top_left = {
                random_int(1, level_size[0] - room_size[0] - 2),
                random_int(1, level_size[1] - room_size[1] - 2)
            },
            .size = {
                room_size[0],
                room_size[1]
            }
        };
        bool overlapping = false;
        for (int32_t room_index = 0; room_index < room_count && !overlapping; room_index++)
            if (rooms_overlapping(&r1, &rooms[room_index]))
                overlapping = true;
        if (overlapping) continue;

        // We found a valid spot
        out_top_left[0] = r1.top_left[0];
        out_top_left[1] = r1.top_left[1];
        return true;
    }
    return false;
}

// Picks a spot in a room that isnt on the edge so you can safely place doors in and out
static void pick_spot_in_room(RoomSpace *r, Position out_spot) {
    out_spot[0] = random_int(r->top_left[0] + 1, r->top_left[0] + r->size[0] - 2);
    out_spot[1] = random_int(r->top_left[1] + 1, r->top_left[1] + r->size[1] - 2);
}

// Same as above but for spawning (ie, can be on the edges
static void pick_spawn_in_room(RoomSpace *r, Position out_spot) {
    out_spot[0] = random_int(r->top_left[0], r->top_left[0] + r->size[0]);
    out_spot[1] = random_int(r->top_left[1], r->top_left[1] + r->size[1]);
}

// Cuts a vertical line in the map
static void carve_hallway_vertical(Oct_Tilemap tilemap, Position p1, Position p2) {
    int32_t i = 0;
    int32_t max = abs(p1[1] - p2[1]);
    int32_t direction = sign(p2[1] - p1[1]);
    while (i <= max) {
        oct_SetTilemap(tilemap, p1[0], p1[1] + (i * direction), get_floor_tile());
        i++;
    }
}

// Cuts a horizontal line in the map
static void carve_hallway_horizontal(Oct_Tilemap tilemap, Position p1, Position p2) {
    int32_t i = 0;
    int32_t max = abs(p1[0] - p2[0]);
    int32_t direction = sign(p2[0] - p1[0]);
    while (i <= max) {
        oct_SetTilemap(tilemap, p1[0]+ (i * direction), p1[1], get_floor_tile());
        i++;
    }
}

// Carves a hallway between the two points (places 0 along the hallway)
static void carve_hallway(Oct_Tilemap tilemap, Position p1, Position p2) {
    carve_hallway_horizontal(tilemap, p1, p2);
    Position new_point = {
            p2[0],
            p1[1]
    };
    carve_hallway_vertical(tilemap, new_point, p2);
}

static bool is_door_location(Oct_Tilemap tilemap, Oct_Tilemap decorations, RoomSpace *rooms, int32_t room_count, int32_t x, int32_t y) {
    for (int32_t i = 0; i < room_count; i++) {
        if (x >= rooms[i].top_left[0] - 1 && x <= rooms[i].top_left[0] + rooms[i].size[0] + 2 &&
            y >= rooms[i].top_left[1] - 1 && y <= rooms[i].top_left[1] + rooms[i].size[1] + 2) {
            const bool above = is_floor_tile(oct_GetTilemap(tilemap, x, y - 1));
            const bool below = is_floor_tile(oct_GetTilemap(tilemap, x, y + 1));
            const bool left = is_floor_tile(oct_GetTilemap(tilemap, x - 1, y));
            const bool right = is_floor_tile(oct_GetTilemap(tilemap, x + 1, y));
            const bool DOOR_ANYWHERE_FFS = oct_GetTilemap(decorations, x + 1, y) == TILE_DOOR_CLOSED ||
                                           oct_GetTilemap(decorations, x - 1, y) == TILE_DOOR_CLOSED ||
                                           oct_GetTilemap(decorations, x, y + 1) == TILE_DOOR_CLOSED ||
                                           oct_GetTilemap(decorations, x, y - 1) == TILE_DOOR_CLOSED;
            if (DOOR_ANYWHERE_FFS || !is_floor_tile(oct_GetTilemap(tilemap, x, y))) return false;
            if ((above && below && !left && !right) || (!above && !below && left && right))
                return true;
        }
    }
    return false;
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

// Finds the center of a room and returns it (returns center_out)
void center_of_room(RoomSpace *r, Position center_out) {
    center_out[0] = r->top_left[0] + (r->size[0] / 2);
    center_out[1] = r->top_left[1] + (r->size[1] / 2);
}

void generate_level(Level *level, LevelGenerationParameters *params, Position out_player_pos) {
    const int32_t spawns_per_room = 5;

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
    Oct_Tilemap tilemap = oct_CreateTilemap(
            oct_GetAsset(g_game.assets, "tileset.png"),
            params->level_size[0], params->level_size[1],
            (Oct_Vec2){CELL_WIDTH, CELL_HEIGHT});
    Oct_Tilemap decorations = oct_CreateTilemap(
            oct_GetAsset(g_game.assets, "tileset.png"),
            params->level_size[0], params->level_size[1],
            (Oct_Vec2){CELL_WIDTH, CELL_HEIGHT});
    int32_t room_count = random_int(params->room_count[0], params->room_count[1] + 1);
    RoomSpace *rooms = oct_Malloc(g_game.allocator, sizeof(RoomSpace) * room_count);
    level->tiles = level_tiles;
    level->tilemap = tilemap;
    level->decorations = decorations;
    level->level_width = params->level_size[0];
    level->level_height = params->level_size[1];

    // Create each room
    int32_t temp_room_count;
    for (temp_room_count = 0; temp_room_count < room_count; temp_room_count++) {
        IntRange current_room_size = {
            random_int(params->room_min_size[0], params->room_max_size[0] + 1),
            random_int(params->room_min_size[1], params->room_max_size[1] + 1)
        };
        Position top_left = {0}; // top left of the new room

        // Attempt to create a room
        if (!find_space_for_room(rooms, temp_room_count, params->level_size, current_room_size, top_left)) {
            // Try to create the room at the minimum possible size
            if (!find_space_for_room(rooms, temp_room_count, params->level_size, params->room_min_size, top_left)) {
                // Clearly there is not much room left
                oct_Raise(OCT_STATUS_ERROR, false, "Ran out of space for more rooms at room[%i].", temp_room_count);
                break;
            }
            // We need the small ahh room
            current_room_size[0] = params->room_min_size[0];
            current_room_size[1] = params->room_min_size[1];
        }

        // One way or another there is space for the room here
        rooms[temp_room_count].top_left[0] = top_left[0];
        rooms[temp_room_count].top_left[1] = top_left[1];
        rooms[temp_room_count].size[0] = current_room_size[0];
        rooms[temp_room_count].size[1] = current_room_size[1];
    }
    room_count = temp_room_count;
    const int32_t start_room = 0;
    const int32_t end_room = room_count - 1;

    // Fill the tilemap with walls wherever there is no room
    for (int32_t y = 0; y < params->level_size[1]; y++) {
        for (int32_t x = 0; x < params->level_size[0]; x++) {
            // Check if this spot overlaps with a room
            oct_SetTilemap(
                    tilemap,
                    x, y,
                    tile_in_room(rooms, room_count, (Position){x, y}) ? get_floor_tile() : get_wall_tile());
        }
    }

    // Connect every room end-to-end
    for (int32_t i = 0; i < room_count - 1; i++) {
        Position p1, p2;
        pick_spot_in_room(&rooms[i], p1);
        pick_spot_in_room(&rooms[i + 1], p2);
        carve_hallway(tilemap, p1, p2);
    }

    // Add random hallways
    const int32_t extra_hallway_count = random_int(params->extra_hallways[0], params->extra_hallways[1] + 1);
    for (int32_t i = 0; i < extra_hallway_count; i++) {
        const int32_t hallway_start_room = random_int(0, room_count);
        int32_t hallway_end_room = random_int(0, room_count);
        while (hallway_end_room == hallway_start_room) hallway_end_room = random_int(0, room_count);
        Position p1, p2;
        pick_spot_in_room(&rooms[hallway_start_room], p1);
        pick_spot_in_room(&rooms[hallway_end_room], p2);
        carve_hallway(tilemap, p1, p2);
    }

    // Place walls where the tileset has no floors
    for (int32_t y = 0; y < params->level_size[1]; y++) {
        for (int32_t x = 0; x < params->level_size[0]; x++) {
            if (!is_floor_tile(oct_GetTilemap(tilemap, x, y))) {
                TileContents *t = level_get_tile((Position) {x, y});
                assert(t);
                t->type = TILE_CONTENTS_TYPE_WALL;
            }
        }
    }

    // Choose a bunch of potential spawn points in every room except for the starting room
    level->spawn_points = oct_Zalloc(g_game.allocator, spawns_per_room * sizeof(Position) * (room_count - 1));
    level->spawn_points_count = (room_count - 1) * spawns_per_room;
    for (int32_t i = 1; i < room_count; i++) {
        for (int32_t j = 0; j < spawns_per_room; j++) {
            bool overlapping = true;
            while (overlapping) {
                pick_spawn_in_room(&rooms[i], level->spawn_points[((i - 1) * spawns_per_room) + j]);

                // Check if this spot already exists in the spawn points
                const int32_t starting_index = (i - 1) * spawns_per_room;
                overlapping = false;
                for (int32_t k = starting_index; k < starting_index + j; k++) {
                    if (level->spawn_points[((i - 1) * spawns_per_room) + j][0] == level->spawn_points[k][0] &&
                        level->spawn_points[((i - 1) * spawns_per_room) + j][1] == level->spawn_points[k][1])
                        overlapping = true;
                }
                if (overlapping)
                    debug("Re-rolling spot [%i,%i]", level->spawn_points[((i - 1) * spawns_per_room) + j][0], level->spawn_points[((i - 1) * spawns_per_room) + j][1]);
            }
        }
    }

    // Place decorations and doors
    for (int32_t y = 0; y < params->level_size[1]; y++) {
        for (int32_t x = 0; x < params->level_size[0]; x++) {
            autotile(tilemap, decorations, x, y);
            if (is_door_location(tilemap, decorations, rooms, room_count, x, y)) {
                oct_SetTilemap(decorations, x, y, TILE_DOOR_CLOSED);
                TileContents *t = level_get_tile((Position){x, y});
                assert(t);
                t->type = TILE_CONTENTS_TYPE_WALL;
                t->tile.door = true;
                t->tile.door_open = false;
            }
        }
    }

    // Place stairs up and down
    Position stairs_down, stairs_up;
    center_of_room(&rooms[start_room], stairs_down);
    center_of_room(&rooms[end_room], stairs_up);
    oct_SetTilemap(tilemap, stairs_up[0], stairs_up[1], TILE_STAIRS_UP);
    oct_SetTilemap(tilemap, stairs_down[0], stairs_down[1], TILE_STAIRS_DOWN);

    // Draw the entire level to a texture
    level->level_tex = oct_CreateSurface((Oct_Vec2){(float)level->level_width * CELL_WIDTH, (float)level->level_height * CELL_HEIGHT});
    assert(level->level_tex);

    // Place the player
    out_player_pos[0] = stairs_down[0];
    out_player_pos[1] = stairs_down[1];
    oct_Free(g_game.allocator, rooms);
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
