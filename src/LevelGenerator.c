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
    return random_int(17, 22);
}

static bool is_floor_tile(int32_t tile) {
    return tile >= 17 && tile <= 21;
}

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
    const bool horizontal_then_vertical = oct_Random(0, 1) > 0.5;
    if (horizontal_then_vertical) {
        carve_hallway_horizontal(tilemap, p1, p2);
        Position new_point = {
                p2[0],
                p1[1]
        };
        carve_hallway_vertical(tilemap, new_point, p2);
    } else {
        carve_hallway_vertical(tilemap, p1, p2);
        Position new_point = {
                p1[0],
                p2[1]
        };
        carve_hallway_horizontal(tilemap, new_point, p2);
    }
}

void generate_level(Level *level, LevelGenerationParameters *params, Position out_player_pos) {
    // stop future me from being a dumbass
    assert(params->room_count[0] > 2 && params->room_count[0] > 2);
    assert(params->room_max_size[0] < params->level_size[0]);
    assert(params->room_max_size[1] < params->level_size[1]);

    // Create resources and pick a room count
    TileContents *level_tiles = oct_Zalloc(
        g_game.allocator,
        sizeof(TileContents) * params->level_size[0] * params->level_size[1]);
    Oct_Tilemap tilemap = oct_CreateTilemap(
        oct_GetAsset(g_game.assets, "tileset.png"),
        params->level_size[0], params->level_size[1],
        (Oct_Vec2){CELL_WIDTH, CELL_HEIGHT});
    int32_t room_count = random_int(params->room_count[0], params->room_count[1] + 1);
    RoomSpace *rooms = oct_Malloc(g_game.allocator, sizeof(RoomSpace) * room_count);
    level->tiles = level_tiles;
    level->tilemap = tilemap;
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

    // Draw the entire level to a texture
    level->level_tex = oct_CreateSurface((Oct_Vec2){(float)level->level_width * CELL_WIDTH, (float)level->level_height * CELL_HEIGHT});
    assert(level->level_tex);
    set_draw_target(level->level_tex);
    oct_DrawClear(&(Oct_Colour){.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f});
    oct_TilemapDraw(tilemap);
    reset_draw_target();

    // Place the player
    pick_spot_in_room(&rooms[start_room], out_player_pos);
    oct_Free(g_game.allocator, rooms);
}

void cleanup_level(Level *level) {
    oct_FreeAsset(level->level_tex);
    oct_Free(g_game.allocator, level->tiles);
    oct_DestroyTilemap(level->tilemap);
}
