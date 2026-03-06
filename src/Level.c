#include <oct/Octarine.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "Game.h"
#include "Util.h"
#include "Character.h"

// Returns true if a given tile is within range of the player's attack range
static inline bool tile_in_range_of_player(Position target) {
    return tile_distance(target, g_game.player.pos) <= g_game.player.weapons[g_game.player.active_weapon].range;
}

void characters_update() {
    // TODO: Enemy turns
    if (g_game.current_level.world_turn && g_game.current_level.state == LEVEL_STATE_ENEMY_TURN) {
        g_game.current_level.world_turn = false;
        g_game.current_level.state = LEVEL_STATE_PLAYER_INTERACTION;
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

        // Characters in attack animations are processed separately
        if (level_in_attack_animation() && (c == g_game.current_level.Attack.attacker || g_game.current_level.Attack.receiver))
            continue;

        if (!character_is_alive(c)) continue;

        // Move character to where they should be
        const Oct_Vec2 velocity = {
                (c_info->target_position[0] - c_info->actual_position[0]) * 0.4f,
                (c_info->target_position[1] - c_info->actual_position[1]) * 0.4f,
        };
        c_info->actual_position[0] += velocity[0];
        c_info->actual_position[1] += velocity[1];

        // Make sure the facing value accurately reflects where the character is pointing
        if (velocity[0] > 0)
            c_info->facing_direction = 1;
        else if (velocity[0] < 0)
            c_info->facing_direction = -1;

        // Cool rotation effect
        const float speed = sqrtf(powf(velocity[0], 2) + powf(velocity[1], 2));
        c_info->rotation = speed * c_info->facing_direction * 0.15f;

        // Lerp scale to match facing
        c_info->scale_x += (c_info->facing_direction - c_info->scale_x) * 0.35f;

        character_draw(c, c_info->actual_position);
    }

    // Draw the attack animation
    if (level_in_attack_animation()) {
        // TODO: This
    }
}

void draw_ui() {
    // TODO: This
}

void draw_tiles() {
    float camera_x, camera_y;
    get_camera_coords(&camera_x, &camera_y, nullptr, nullptr);
    const int32_t start_draw_x = (int32_t)floorf((camera_x - CELL_WIDTH) / CELL_WIDTH);
    const int32_t start_draw_y = (int32_t)floorf((camera_y - CELL_HEIGHT) / CELL_HEIGHT);
    const int32_t tile_horizontal = (int32_t)ceilf((GAME_VIEW_WIDTH + (CELL_WIDTH * 2)) / CELL_WIDTH) + 1;
    const int32_t tile_vertical = (int32_t)ceilf((GAME_VIEW_WIDTH + (CELL_WIDTH * 2)) / CELL_WIDTH) + 1;

    oct_TilemapDrawPart(g_game.current_level.tilemap, start_draw_x, start_draw_y, tile_horizontal, tile_vertical);
}

void draw_attack_view() {
    if (g_game.current_level.state != LEVEL_STATE_PLAYER_ATTACK) return;
    g_game.current_level.attack_view.cursor_real_pos[0] += ((g_game.current_level.attack_view.attack_cursor[0] * CELL_WIDTH) - g_game.current_level.attack_view.cursor_real_pos[0]) * 0.4f;
    g_game.current_level.attack_view.cursor_real_pos[1] += ((g_game.current_level.attack_view.attack_cursor[1] * CELL_HEIGHT) - g_game.current_level.attack_view.cursor_real_pos[1]) * 0.4f;
    const Oct_Vec2 cursor_pos_real = {
            g_game.current_level.attack_view.cursor_real_pos[0],
            g_game.current_level.attack_view.cursor_real_pos[1]
    };
    uint64_t id = ATTACK_CURSOR_ID_START;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            Oct_Texture texture = OCT_NO_ASSET;
            const bool tile_in_range = tile_in_range_of_player((Position){
                    x + g_game.current_level.attack_view.attack_cursor[0],
                    y + g_game.current_level.attack_view.attack_cursor[1]});
            const bool selected_tile = x == 0 && y == 0;
            if (x + g_game.current_level.attack_view.attack_cursor[0] == g_game.player.pos[0] && y + g_game.current_level.attack_view.attack_cursor[1] == g_game.player.pos[1]) {
                id++;
                continue;
            }
            if (tile_in_range && selected_tile)
                texture = oct_GetAsset(g_game.assets, "hud/green_tile.png");
            else if (tile_in_range && !selected_tile)
                texture = oct_GetAsset(g_game.assets, "hud/green_tile_small.png");
            else if (!tile_in_range && selected_tile)
                texture = oct_GetAsset(g_game.assets, "hud/red_tile.png");
            else // (!tile_in_range && !selected_tile)
                texture = oct_GetAsset(g_game.assets, "hud/red_tile_small.png");
            oct_DrawTextureInt(
                    OCT_INTERPOLATE_ALL, id,
                    texture,
                    (Oct_Vec2){cursor_pos_real[0] + ((float)x * CELL_WIDTH),
                               cursor_pos_real[1] + ((float)y * CELL_HEIGHT)});
            id++;
        }
    }
}

// Draws the portion of the attack view that needs to be in the UI and not game world
void draw_attack_view_ui() {
    static char buffer[50];
    static const int32_t max_buffer_len = 49;
    if (g_game.current_level.state != LEVEL_STATE_PLAYER_ATTACK) return;

    const float SCREEN_X = 10;
    const float SCREEN_Y = 10;

    // Show roll stats if cursor is on something
    const TileContents *contents = level_get_tile(g_game.current_level.attack_view.attack_cursor);
    if (contents && (contents->type == TILE_CONTENTS_TYPE_CHARACTER || contents->type == TILE_CONTENTS_TYPE_ITEM)) {
        if (contents->type == TILE_CONTENTS_TYPE_CHARACTER && contents->character == &g_game.player) return;
        const Traits *target_traits = contents->type == TILE_CONTENTS_TYPE_CHARACTER ?
                                      &contents->character->info.traits :
                                      &contents->item->info.traits;
        int32_t dc, pips;
        const AttackFavour favour =  character_get_attack_stats(&g_game.player,
                                                                &g_game.player.weapons[g_game.player.active_weapon].info.traits,
                                                                g_game.current_level.attack_view.attack_cursor,
                                                                target_traits,
                                                                &pips, &dc);
        const char *favoured = "";
        if (favour == ATTACK_FAVOUR_GOOD) favoured = GLYPH_UP;
        if (favour == ATTACK_FAVOUR_BAD) favoured = GLYPH_DOWN;

        // Get text size to properly position the overlay
        snprintf(buffer, max_buffer_len, "1%s%i%s%s%s%i", GLYPH_D8, pips, GLYPH_D6, favoured, GLYPH_POINT_RIGHT, dc);
        Oct_Vec2 text_size = {(float)strlen(buffer) * 12, 14};

        oct_DrawTexture(oct_GetAsset(g_game.assets, "hud/predictionbackground.png"),
                        (Oct_Vec2){SCREEN_X, SCREEN_Y});
        oct_DrawTextColour(
                oct_GetAsset(g_game.assets, "fnt_dice"),
                (Oct_Vec2){SCREEN_X + 2, SCREEN_Y + 2},
                &(Oct_Colour){.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f}, 1,
                "%s", buffer);
    }
}

void draw_labels() {
    for (int i = 0; i < MAX_LABELS; i++) {
        if (g_game.current_level.labels[i].ticks_remaining > 0) {
            const float alpha = (float)g_game.current_level.labels[i].ticks_remaining / (float)g_game.current_level.labels[i].max_ticks;
            Oct_Colour c = g_game.current_level.labels[i].colour;
            c.a = alpha;
            const Oct_FontAtlas font = g_game.current_level.labels[i].dice_font ?
                                       oct_GetAsset(g_game.assets, "fnt_dice") :
                                       oct_GetAsset(g_game.assets, "fnt_small");
            oct_DrawTextIntColour(
                    OCT_INTERPOLATE_ALL, LABELS_ID_START + i,
                    font,
                    g_game.current_level.labels[i].position,
                    &c, 1,
                    "%s", g_game.current_level.labels[i].label);
            if (g_game.current_level.labels[i].dice_font)
                g_game.current_level.labels[i].position[1] -= 0.15f;
            else
                g_game.current_level.labels[i].position[1] -= 0.3f;

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

    // Tilemap
    g_game.current_level.tilemap = oct_CreateTilemap(
            oct_GetAsset(g_game.assets, "tileset.png"),
            100, 100,
            (Oct_Vec2){CELL_WIDTH, CELL_HEIGHT});
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            const int32_t tile = random_int(17, 17 + 5);
            oct_SetTilemap(g_game.current_level.tilemap, x, y, tile);
        }
    }
    g_game.current_level.level_width = 100;
    g_game.current_level.level_height = 100;
    g_game.current_level.tiles = oct_Zalloc(g_game.allocator, sizeof(TileContents) * g_game.current_level.level_width * g_game.current_level.level_height);

    player_init();

    create_slime(level_get_character_slot(), (Position){15, 15});
}

LevelIndex level_update() {
    const float window_width = oct_WindowWidth();
    const float window_height = oct_WindowHeight();
    float camera_x, camera_y, camera_w, camera_h;
    get_camera_coords(&camera_x, &camera_y, &camera_w, &camera_h);
    Oct_CameraUpdate camera_update = {
            .size = {camera_w, camera_h},
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
    draw_attack_view();
    draw_labels();

    // UI drawing
    oct_LockCameras(g_game.ui_camera);
    draw_attack_view_ui();
    draw_ui();

    timer_tick(&g_game.current_level.Attack.animation_timer);
    return g_game.level_index;
}

void level_end() {
    oct_DestroyTilemap(g_game.current_level.tilemap);
    oct_Free(g_game.allocator, g_game.current_level.tiles);
}

void create_label(const char *text, const Position pos, Oct_Colour colour, bool needs_to_be_freed) {
    for (int i = 0; i < MAX_LABELS; i++) {
        if (g_game.current_level.labels[i].ticks_remaining <= 0) {
            g_game.current_level.labels[i].colour = colour;
            g_game.current_level.labels[i].ticks_remaining = 30;
            g_game.current_level.labels[i].max_ticks = 30;
            g_game.current_level.labels[i].needs_to_be_freed = needs_to_be_freed;
            g_game.current_level.labels[i].label = text;
            g_game.current_level.labels[i].position[0] = ((float)pos[0] * CELL_WIDTH) - ((float)strlen(text) * 6.0f * 0.5f);
            g_game.current_level.labels[i].position[1] = ((float)pos[1] * CELL_HEIGHT);
            g_game.current_level.labels[i].dice_font = false;
            return;
        }
    }
}

void create_dice_label(const char *text, const Position pos, Oct_Colour colour, bool needs_to_be_freed) {
    for (int i = 0; i < MAX_LABELS; i++) {
        if (g_game.current_level.labels[i].ticks_remaining <= 0) {
            g_game.current_level.labels[i].colour = colour;
            g_game.current_level.labels[i].ticks_remaining = 60;
            g_game.current_level.labels[i].max_ticks = 60;
            g_game.current_level.labels[i].needs_to_be_freed = needs_to_be_freed;
            g_game.current_level.labels[i].label = text;
            g_game.current_level.labels[i].position[0] = ((float)pos[0] * CELL_WIDTH) - ((float)strlen(text) * 11.0f * 0.5f);
            g_game.current_level.labels[i].position[1] = ((float)pos[1] * CELL_HEIGHT);
            g_game.current_level.labels[i].dice_font = true;
            return;
        }
    }
}

Character *level_get_character_slot() {
    for (int i = 0; i < MAX_CHARACTERS; i++)
        if (!character_is_alive(&g_game.current_level.characters[i]))
            return &g_game.current_level.characters[i];

    oct_Raise(OCT_STATUS_ERROR, true, "Ran out of character slots.");
    return nullptr;
}

TileContents *level_get_tile(Position pos) {
    if (pos[0] < 0 || pos[0] >= g_game.current_level.level_width || pos[1] < 0 || pos[1] >= g_game.current_level.level_height)
        return nullptr;
    return &g_game.current_level.tiles[(pos[1] * g_game.current_level.level_width) + pos[0]];
}

Traits *level_get_tile_traits(Position pos) {
    const TileContents *t = level_get_tile(pos);
    if (t && t->type == TILE_CONTENTS_TYPE_CHARACTER) return &t->character->info.traits;
    if (t && t->type == TILE_CONTENTS_TYPE_ITEM) return &t->item->info.traits;
    return nullptr;
}

TileContentsType level_get_tile_type(int32_t x, int32_t y) {
    if (x < 0 || x >= g_game.current_level.level_width || y < 0 || y >= g_game.current_level.level_height)
        return TILE_CONTENTS_TYPE_NONE;
    return g_game.current_level.tiles[(y * g_game.current_level.level_width) + x].type;
}

bool level_in_attack_animation() {
    return timer_in_use(&g_game.current_level.Attack.animation_timer);
}

bool level_attack_animation_complete() {
    return timer_is_done(&g_game.current_level.Attack.animation_timer);
}
