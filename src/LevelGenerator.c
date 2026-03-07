#include "LevelGenerator.h"
#include "Util.h"
#include "Game.h"

// For keeping tracks of room in the dungeon while level generating
typedef struct RoomSpace_s {
    Position top_left;
    IntRange size;
} RoomSpace;

// Returns true if two rooms are overlapping or their borders are touching
bool rooms_overlapping(RoomSpace *r1, RoomSpace *r2) {
    if (r1->top_left[0] - 1 <= r2->top_left[0] + r2->size[0] + 2 &&
        r1->top_left[1] - 1 <= r2->top_left[1] + r2->size[1] + 2 &&
        r1->top_left[0] + r1->size[0] + 2 >= r2->top_left[0] - 1 &&
        r1->top_left[1] + r1->size[1] + 2 >= r2->top_left[1] - 1)
        return true;
    return false;
}

// Returns false if it fails to find a spot for that room in 10 attempts
bool find_space_for_room(RoomSpace *rooms, int32_t room_count, IntRange level_size, IntRange room_size, Position out_top_left) {
    for (int32_t i = 0; i < 10; i++) {
        RoomSpace r1 = {
            .top_left = {
                random_int(0, level_size[0] - room_size[0]),
                random_int(0, level_size[1] - room_size[1])
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

void generate_level(Level *level, LevelGenerationParameters *params) {
    // Create resources and pick a room count
    TileContents *level_tiles = oct_Malloc(
        g_game.allocator,
        sizeof(TileContents) * params->level_size[0] * params->level_size[1]);
    Oct_Tilemap tilemap = oct_CreateTilemap(
        oct_GetAsset(g_game.assets, "tileset.png"),
        params->level_size[0], params->level_size[1],
        (Oct_Vec2){CELL_WIDTH, CELL_HEIGHT});
    const int32_t room_count = random_int(params->room_count[0], params->room_count[1] + 1);
    RoomSpace *rooms = oct_Malloc(g_game.allocator, sizeof(RoomSpace) * room_count);

    // Create each room
    for (int32_t i = 0; i < room_count; i++) {
        IntRange current_room_size = {
            random_int(params->room_min_size[0], params->room_max_size[0] + 1),
            random_int(params->room_min_size[1], params->room_max_size[1] + 1)
        };

        // Attempt to create a room, if it fails, drop to lowest possible room size and try again
    }

    oct_Free(g_game.allocator, rooms);
}

void cleanup_level(Level *level) {
    oct_Free(g_game.allocator, level->tiles);
    oct_DestroyTilemap(level->tilemap);
}
