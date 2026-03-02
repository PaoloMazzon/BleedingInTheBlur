#include <oct/Octarine.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "Game.h"
#include "Util.h"
#include "Character.h"

void player_update() {
    Character *player = &g_game.player;

    // Should only be 1/-1 in both directions
    Position movement_direction = {0};
    if (oct_KeyPressed(OCT_KEY_LEFT)) movement_direction[0] = -1;
    else if (oct_KeyPressed(OCT_KEY_RIGHT)) movement_direction[0] = 1;
    else if (oct_KeyPressed(OCT_KEY_UP)) movement_direction[1] = -1;
    else if (oct_KeyPressed(OCT_KEY_DOWN)) movement_direction[1] = 1;

    else if (oct_KeyPressed(OCT_KEY_SPACE))
        create_label("fuck you", player->pos, (Oct_Colour){1, 1, 1, 1}, false);

    // Player is considered to have taken their turn if they actually did something
    if (movement_direction[0] != 0 || movement_direction[1] != 0) {
        // World gets to take a turn unless player has accumulated enough movement to get
        // an extra turn
        if (g_game.player.cumulative_movement == 100) {
            g_game.player.cumulative_movement = 0;
        } else {
            g_game.current_level.world_turn = true;
        }
        g_game.player.cumulative_movement += oct_Clampi(0, 100, character_movement(&g_game.player));

        // Process movement
        const Position target_position = {
                g_game.player.pos[0] + movement_direction[0],
                g_game.player.pos[1] + movement_direction[1],
        };
        character_move(&g_game.player, target_position);
    }
}

void characters_update() {
    if (g_game.current_level.world_turn) {
        g_game.current_level.world_turn = false;
        // TODO: This
    }
}

// This will also move characters to where they are supposed to be in the game world
void draw_characters() {
    for (int i = 0; i < MAX_CHARACTERS + 1; i++) {
        // Accounts for player as the last character to draw
        Character *c = nullptr;
        if (i == MAX_CHARACTERS) c = &g_game.player;
        else c = &g_game.current_level.characters[i];
        ObjectInfo *c_info = &c->info;

        if (!character_is_alive(c)) continue;

        // Move character to where they should be
        c_info->actual_position[0] += (c_info->target_position[0] - c_info->actual_position[0]) * 0.4f;
        c_info->actual_position[1] += (c_info->target_position[1] - c_info->actual_position[1]) * 0.4f;

        character_draw(c, c_info->actual_position);
    }
}

void draw_ui() {
    // TODO: This
}

void draw_tiles() {
    float camera_x, camera_y;
    get_camera_coords(&camera_x, &camera_y);
    const int32_t start_draw_x = (int32_t)floorf((camera_x - CELL_WIDTH) / CELL_WIDTH);
    const int32_t start_draw_y = (int32_t)floorf((camera_y - CELL_HEIGHT) / CELL_HEIGHT);
    const int32_t tile_horizontal = (int32_t)ceilf((GAME_VIEW_WIDTH + (CELL_WIDTH * 2)) / CELL_WIDTH);
    const int32_t tile_vertical = (int32_t)ceilf((GAME_VIEW_WIDTH + (CELL_WIDTH * 2)) / CELL_WIDTH);

    oct_TilemapDrawPart(g_game.current_level.tilemap, start_draw_x, start_draw_y, tile_horizontal, tile_vertical);
}

void draw_labels() {
    for (int i = 0; i < MAX_LABELS; i++) {
        if (g_game.current_level.labels[i].ticks_remaining > 0) {
            oct_DrawTextColour(
                    oct_GetAsset(g_game.assets, "fnt_pixel"),
                    g_game.current_level.labels[i].position,
                    &g_game.current_level.labels[i].colour, 1,
                    "%s", g_game.current_level.labels[i].label);
            g_game.current_level.labels[i].position[1] -= 1;

            g_game.current_level.labels[i].ticks_remaining -= 1;
            if (g_game.current_level.labels[i].ticks_remaining == 0 && g_game.current_level.labels[i].needs_to_be_freed) {
                oct_Free(g_game.allocator, (void*)g_game.current_level.labels[i].label);
                g_game.current_level.labels[i].label = nullptr;
            }
        }
    }
}

void level_begin() {
    memset(&g_game.current_level, 0, sizeof(Level));

    Statblock sb;
    random_statblock(&sb);
    print_statblock(&sb);
    character_create(&sb, &g_game.player);
    g_game.player.info.drawn_type = DRAWN_TYPE_TEXTURE;
    g_game.player.info.texture = oct_GetAsset(g_game.assets, "player.png");

    // Tilemap
    g_game.current_level.tilemap = oct_CreateTilemap(
            oct_GetAsset(g_game.assets, "tileset.png"),
            100, 100,
            (Oct_Vec2){CELL_WIDTH, CELL_HEIGHT});
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            oct_SetTilemap(g_game.current_level.tilemap, x, y, ((x + y) % 5) + 17);
        }
    }
}

LevelIndex level_update() {
    const float window_width = oct_WindowWidth();
    const float window_height = oct_WindowHeight();
    float camera_x, camera_y;
    get_camera_coords(&camera_x, &camera_y);
    Oct_CameraUpdate camera_update = {
            .size = {GAME_VIEW_WIDTH, GAME_VIEW_HEIGHT},
            .screenSize = {VIRTUAL_WIDTH, VIRTUAL_HEIGHT},
            .screenPosition = {0, 0},
            .position = {camera_x, camera_y}
    };
    oct_UpdateCameraInt(OCT_INTERPOLATE_ALL, CAMERA_ID, g_game.world_camera, &camera_update);

    // Update logic/turn logic
    player_update();
    update_camera_coords();
    characters_update();

    // Drawing the world
    oct_LockCameras(g_game.world_camera);
    draw_tiles();
    draw_characters();
    draw_labels();

    // UI drawing
    oct_LockCameras(g_game.ui_camera);
    draw_ui();

    return g_game.level_index;
}

void level_end() {
    oct_DestroyTilemap(g_game.current_level.tilemap);
}

void create_label(const char *text, Position pos, Oct_Colour colour, bool needs_to_be_freed) {
    for (int i = 0; i < MAX_LABELS; i++) {
        if (g_game.current_level.labels[i].ticks_remaining <= 0) {
            g_game.current_level.labels[i].colour = colour;
            g_game.current_level.labels[i].ticks_remaining = 30;
            g_game.current_level.labels[i].max_ticks = 30;
            g_game.current_level.labels[i].needs_to_be_freed = needs_to_be_freed;
            g_game.current_level.labels[i].label = text;
            g_game.current_level.labels[i].position[0] = (float)pos[0] * CELL_WIDTH - ((float)strlen(text) * 7.0f * 0.5f);
            g_game.current_level.labels[i].position[1] = (float)pos[1] * CELL_HEIGHT;
        }
    }
}

Character *level_get_character_slot() {
    for (int i = 0; i < MAX_CHARACTERS; i++)
        if (!character_is_alive(&g_game.current_level.characters[i]))
            return &g_game.current_level.characters[i];

    // This should never happen and if it does, increase max character count
    assert(false);
    return nullptr;
}

TileContents *level_get_tile(int32_t x, int32_t y) {
    if (x < 0 || x >= g_game.current_level.level_width || y < 0 || y >= g_game.current_level.level_height)
        return nullptr;
    return &g_game.current_level.tiles[(y * g_game.current_level.level_width) + x];
}
