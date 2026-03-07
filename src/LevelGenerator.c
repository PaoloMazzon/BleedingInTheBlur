#include <stdio.h>
#include "LevelGenerator.h"
#include "Util.h"
#include "Game.h"

// For keeping tracks of room in the dungeon while level generating
typedef struct RoomSpace_s {
    Position top_left;
    IntRange size;
} RoomSpace;

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
    int32_t room_count = random_int(params->room_count[0], params->room_count[1] + 1);
    RoomSpace *rooms = oct_Malloc(g_game.allocator, sizeof(RoomSpace) * room_count);

    // Create each room
    int32_t i;
    for (i = 0; i < room_count; i++) {
        IntRange current_room_size = {
            random_int(params->room_min_size[0], params->room_max_size[0] + 1),
            random_int(params->room_min_size[1], params->room_max_size[1] + 1)
        };
        Position top_left = {0}; // top left of the new room

        // Attempt to create a room
        if (!find_space_for_room(rooms, i, params->level_size, current_room_size, top_left)) {
            // Try to create the room at the minimum possible size
            if (!find_space_for_room(rooms, i, params->level_size, params->room_min_size, top_left)) {
                // Clearly there is not much room left
                oct_Raise(OCT_STATUS_ERROR, false, "Ran out of space for more rooms at room[%i].", i);
                break;
            }
            // We need the small ahh room
            current_room_size[0] = params->room_min_size[0];
            current_room_size[1] = params->room_min_size[1];
        }

        // One way or another there is space for the room here
        rooms[i].top_left[0] = top_left[0];
        rooms[i].top_left[1] = top_left[1];
        rooms[i].size[0] = current_room_size[0];
        rooms[i].size[1] = current_room_size[1];
    }
    room_count = i;

    // TODO: Hallways

    // Fill the tilemap with walls wherever there is no room
    for (int32_t y = 0; y < params->level_size[1]; y++) {
        for (int32_t x = 0; x < params->level_size[0]; x++) {
            // Check if this spot overlaps with a room
            oct_SetTilemap(
                tilemap,
                x, y,
                tile_in_room(rooms, room_count, (Position){x, y}) ? 0 : 1);
            printf("%c", oct_GetTilemap(tilemap, x, y) ? ' ' : '#');
        }
        printf("\n");
    }

    oct_Free(g_game.allocator, rooms);
}

void cleanup_level(Level *level) {
    oct_Free(g_game.allocator, level->tiles);
    oct_DestroyTilemap(level->tilemap);
}
