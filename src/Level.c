#include <oct/Octarine.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "Game.h"
#include "Util.h"
#include "Character.h"
#include "LevelGenerator.h"
#include "WeaponItem.h"
#include "AttackAnimations.h"
#include "PersistentEffects.h"

// Returns true if a given tile is within range of the player's attack range
static inline bool tile_in_range_of_player(Position target) {
    return tile_distance(target, g_game.player.pos) <= get_player_current_attack_range();
}

bool tiles_have_walls_between(Position tile1, Position tile2) {
    int x0 = tile1[0];
    int y0 = tile1[1];
    int x1 = tile2[0];
    int y1 = tile2[1];

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (x0 != x1 || y0 != y1) {
        if (x0 != tile1[0] || y0 != tile1[1]) {
            TileContents *t = level_get_tile((Position){x0, y0});
            if (t == NULL || t->type == TILE_CONTENTS_TYPE_WALL) {
                return true;
            }
        }

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }

    return false;
}

// returns true if you should call it again
bool characters_update() {
    // FIXME: This should be made into a proper state machine. There are only 4 states: waiting to take turn (timer), taking turn (timer), in attack animation, take turn instantly
    if (g_game.current_level.world_turn && g_game.current_level.state == LEVEL_STATE_ENEMY_TURN) {
        Character *enemy = &g_game.current_level.characters[g_game.current_level.enemy_turn];

        // Delayed turn is done
        if (timer_tick(&g_game.current_level.enemy_move_timer) && !level_in_attack_animation()) {
            level_next_enemy_turn();
            return true;
        }

        const bool done_turn_delay = timer_tick(&g_game.current_level.enemy_delayed_turn_timer);
        if (done_turn_delay) {
            character_take_turn(enemy);
            timer_start(&g_game.current_level.enemy_move_timer, 30 / 2);
        } else if (!timer_in_use(&g_game.current_level.enemy_delayed_turn_timer) && !level_in_attack_animation() && !timer_in_use(&g_game.current_level.enemy_move_timer)) {
            if (tile_distance(enemy->pos, g_game.player.pos) > 5 || tiles_have_walls_between(enemy->pos, g_game.player.pos)) {
                // Take turn instantly
                character_take_turn(enemy);
                level_next_enemy_turn();
                return true;
            } else {
                timer_start(&g_game.current_level.enemy_delayed_turn_timer, 30 / 2);
            }
        } else if (!level_in_attack_animation() && (timer_in_use(&g_game.current_level.enemy_move_timer) || timer_in_use(&g_game.current_level.enemy_delayed_turn_timer))) {
            // Center camera on the enemy thats moving around
            look_at(enemy->pos, 1);
        } else if (level_attack_animation_complete()) {
            level_next_enemy_turn();
        }
    } else if (!g_game.current_level.world_turn && g_game.current_level.state == LEVEL_STATE_ENEMY_TURN) {
        g_game.current_level.state = LEVEL_STATE_PLAYER_INTERACTION;
    }
    return false;
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
        const float target_alpha = level_tile_seen_this_turn(c->pos) ? 1 : 0;
        c_info->actual_alpha += (target_alpha - c_info->actual_alpha) * 0.4;

        // Move character to where they should be
        const Oct_Vec2 velocity = {
                (c_info->target_position[0] - c_info->actual_position[0]) * 0.4f,
                (c_info->target_position[1] - c_info->actual_position[1]) * 0.4f,
        };
        c_info->actual_position[0] += velocity[0];
        c_info->actual_position[1] += velocity[1];

        // Make sure the facing value accurately reflects where the character is pointing
        if (!timer_in_use(&c->face_away_timer)) {
            if (velocity[0] > 0.01)
                c_info->facing_direction = 1;
            else if (velocity[0] < -0.01)
                c_info->facing_direction = -1;

            // Lerp scale to match facing
            c_info->scale_x += (c_info->facing_direction - c_info->scale_x) * 0.35f;
        }

        // Cool rotation effect
        const float speed = sqrtf(powf(velocity[0], 2) + powf(velocity[1], 2));
        c_info->rotation = speed * c_info->facing_direction * 0.15f;

        character_draw(c, c_info->actual_position, c_info->actual_alpha);
        timer_tick(&c->face_away_timer);
    }

    // Draw the attack animation
    if (level_in_attack_animation()) {
        draw_attack_animation();
    }
}

void draw_ui() {
    static float actual_weapon_indicator_offset = 0;
    static float actual_movement_scale = 0;
    const Oct_FontAtlas pretty_font = oct_GetAsset(g_game.assets, "fnt_pixel");

    // We process this all the time
    const float target_item_bar_y = timer_in_use(&g_game.current_level.player_item_bar_popup_timer) ? 0 : 32;
    g_game.current_level.player_item_bar_actual_y += (target_item_bar_y - g_game.current_level.player_item_bar_actual_y) * 0.4f;

    // We want the item popup to draw below the rest of the hud so it can drop behind it
    // item location is 232, 232
    timer_tick(&g_game.current_level.player_item_bar_popup_timer);
    if (g_game.current_level.player_item_bar_actual_y <= 31.5) {
        const float item_bar_normalized_timer = timer_get_normalized(&g_game.current_level.player_item_bar_popup_timer);
        // TODO: This should not be based off of the timer and instead a moving target_y like everything else in case the timer is restarted multiple times (in the case of the player spamming item swap)
        const float y_displacement = g_game.current_level.player_item_bar_actual_y;
        const float item_popup_start_x = 96;
        const float item_popup_start_y = 194 + y_displacement;
        const float item_start_x = 104;
        const float item_start_y = 201 + y_displacement;
        const float item_displacement_x = 32;
        g_game.player.actual_selected += ((float)g_game.player.selected_item - g_game.player.actual_selected) * 0.4;
        const float selected_x = 99 + (item_displacement_x * g_game.player.actual_selected);
        const float selected_y = 196;

        oct_DrawTextureInt(
                OCT_INTERPOLATE_ALL, ITEM_BACKGROUND_ID,
                oct_GetAsset(g_game.assets, "hud/itembackground.png"),
                (Oct_Vec2){item_popup_start_x, item_popup_start_y});

        // Draw each item in the inventory
        for (int32_t i = 0; i < INVENTORY_SIZE; i++) {
            if (g_game.player.items[i].type == ITEM_TYPE_NONE) continue;
            draw_item(
                &g_game.player.items[i],
                (Oct_Vec2){item_start_x + (item_displacement_x * (float)i), item_start_y}, 1);
        }
        // Draw the selector thing over the selected item
        oct_DrawTextureInt(
            OCT_INTERPOLATE_ALL, ITEM_SELECTOR_ID,
            oct_GetAsset(g_game.assets, "hud/weaponselect.png"),
                    (Oct_Vec2){selected_x, selected_y + y_displacement});

        // Draw text for the item name
        if (g_game.player.items[g_game.player.selected_item].type != ITEM_TYPE_NONE) {
            Oct_Vec2 text_size;
            oct_GetTextSize(pretty_font, text_size, 1, "%s",
                            g_game.player.items[g_game.player.selected_item].info.name);
            oct_DrawTextIntColour(
                    OCT_INTERPOLATE_ALL, ITEM_NAME_ID,
                    pretty_font,
                    (Oct_Vec2) {item_popup_start_x + 64 - (text_size[0] / 2), item_popup_start_y - text_size[1]},
                    &(Oct_Colour) {.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f - (y_displacement / 32.0f)},
                    1, "%s", g_game.player.items[g_game.player.selected_item].info.name);
        }
    }

    oct_DrawTexture(
            oct_GetAsset(g_game.assets, "hud/hudbase.png"),
            (Oct_Vec2){0, 224});

    // Stats
    const int32_t max_hp = character_max_hp(&g_game.player);
    const int32_t hp = g_game.player.current_hp;
    const int32_t max_mana = character_max_mana(&g_game.player);
    const int32_t mana = g_game.player.current_mana;
    const int32_t movement = g_game.player.cumulative_movement;

    // Draw HP depending on if the player has above or below 40
    float hp_horizontal_jump = 6;
    float hp_vertical_jump = 4;
    int32_t hp_extra_pixel_at = 5;
    Oct_Texture hp_pip_empty = oct_GetAsset(g_game.assets, "hud/healthpipempty.png");
    Oct_Texture hp_pip_full = oct_GetAsset(g_game.assets, "hud/healthpipfull.png");
    const float hp_start_x = 2;
    const float hp_start_y = 226;
    if (max_hp > 40) {
        hp_horizontal_jump = 3;
        hp_extra_pixel_at = 10;
        hp_pip_empty = oct_GetAsset(g_game.assets, "hud/healthpipempty_big.png");
        hp_pip_full = oct_GetAsset(g_game.assets, "hud/healthpipfull_big.png");
    }
    for (int32_t i = 0; i < max_hp; i++) {
        const float vertical_jump = (float)(i / (hp_extra_pixel_at * 2)) * hp_vertical_jump;
        const float horizontal_jump = ((float)(i % (hp_extra_pixel_at * 2)) * hp_horizontal_jump) + (float)((i % (hp_extra_pixel_at * 2)) / hp_extra_pixel_at);
        oct_DrawTexture(
                hp > i ? hp_pip_full : hp_pip_empty,
                (Oct_Vec2){hp_start_x + horizontal_jump, hp_start_y + vertical_jump});
    }
    // Mana
    const float mana_horizontal_jump = 3;
    const float mana_vertical_jump = 4;
    const int32_t mana_extra_pixel_at = 10;
    const Oct_Texture mana_pip_empty = oct_GetAsset(g_game.assets, "hud/manapipempty.png");
    const Oct_Texture mana_pip_full = oct_GetAsset(g_game.assets, "hud/manapipfull.png");
    const float mana_start_x = 66;
    const float mana_start_y = 226;
    for (int32_t i = 0; i < max_mana; i++) {
        const float vertical_jump = (float)(i / (mana_extra_pixel_at * 2)) * mana_vertical_jump;
        const float horizontal_jump = ((float)(i % (mana_extra_pixel_at * 2)) * mana_horizontal_jump) + (float)((i % (mana_extra_pixel_at * 2)) / mana_extra_pixel_at);
        oct_DrawTexture(
                mana > i ? mana_pip_full : mana_pip_empty,
                (Oct_Vec2){mana_start_x + horizontal_jump, mana_start_y + vertical_jump});
    }

    // movement bar starts at 132, 230  + 17 + 1
    const float player_movement = (float)g_game.player.cumulative_movement / 100.0f;
    actual_movement_scale += (player_movement - actual_movement_scale) * 0.4f;
    oct_DrawTextureIntExt(
            OCT_INTERPOLATE_ALL, MOVEMENT_BAR_ID,
            oct_GetAsset(g_game.assets, "hud/movementbar.png"),
            (Oct_Vec2){132, 230},
            (Oct_Vec2){actual_movement_scale, 1},
            0, (Oct_Vec2){0, 0});
    if (level_extra_player_turn())
        oct_DrawTexture(
                oct_GetAsset(g_game.assets, "hud/extraturn.png"),
                (Oct_Vec2){132 + 17, 230 + 1});

    // weapon highlight is 259 + (32 * index), 227
    if (g_game.player.starting_weapon.type != WEAPON_TYPE_NONE) {
        oct_DrawTexture(g_game.player.starting_weapon.icon, (Oct_Vec2){264, 232});
    }
    if (g_game.player.soul_bound_weapon.type != WEAPON_TYPE_NONE) {
        oct_DrawTexture(g_game.player.soul_bound_weapon.icon, (Oct_Vec2){264 + 32, 232});
    }
    actual_weapon_indicator_offset += (((float)(g_game.player.active_weapon) * 32) - actual_weapon_indicator_offset) * 0.4f;
    oct_DrawTextureInt(
            OCT_INTERPOLATE_ALL, WEAPON_INDICATOR_ID,
            oct_GetAsset(g_game.assets, "hud/weaponselect.png"),
                   (Oct_Vec2){259 + actual_weapon_indicator_offset, 227});

    // Draw the currently selected item
    if (g_game.player.items[g_game.player.selected_item].type != ITEM_TYPE_NONE)
        draw_item_no_int(
                &g_game.player.items[g_game.player.selected_item],
                (Oct_Vec2){232, 232}, 1);

    // Draw enemy info if one needs to be displayed
    timer_tick(&g_game.current_level.enemy_display_timer);
    if (timer_in_use(&g_game.current_level.enemy_display_timer) && g_game.current_level.enemy_displayed) {
        const float normalized = timer_get_normalized(&g_game.current_level.enemy_display_timer);
        const float fade_out_threshold = 0.2f;
        const float alpha = fade_out_threshold >= normalized ? normalized / fade_out_threshold : 1;
        Oct_Colour c = {.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = alpha};
        const float enemy_health = oct_Clamp(0, 1, (float)g_game.current_level.enemy_displayed->current_hp / (float)character_max_hp(g_game.current_level.enemy_displayed));
        g_game.current_level.actual_displayed_health += (enemy_health - g_game.current_level.actual_displayed_health) * 0.4f;
        Oct_Vec2 size = {0};
        oct_GetTextSize(oct_GetAsset(g_game.assets, "fnt_pixel"), size, 1, g_game.current_level.enemy_displayed->info.name);
        oct_DrawTextIntColour(
                OCT_INTERPOLATE_ALL, ENEMY_HUD_NAME_ID,
                oct_GetAsset(g_game.assets, "fnt_pixel"),
                (Oct_Vec2){(VIRTUAL_WIDTH / 2) - (size[0] / 2), 7},
                &c,
                1,
                g_game.current_level.enemy_displayed->info.name);
        oct_DrawTextureIntColour(
                OCT_INTERPOLATE_ALL, ENEMY_HUD_HEALTHBAR_BACKGROUND_ID,
                oct_GetAsset(g_game.assets, "hud/enemyhealthbarbg.png"),
                &c,
                (Oct_Vec2){(VIRTUAL_WIDTH / 2) - (82 / 2), 20});
        Oct_DrawCommand cmd = {
                .type = OCT_DRAW_COMMAND_TYPE_TEXTURE,
                .colour = c,
                .id = ENEMY_HUD_HEALTHBAR_ID,
                .interpolate = OCT_INTERPOLATE_ALL,
                .Texture = {
                        .texture = oct_GetAsset(g_game.assets, "hud/enemyhealthbar.png"),
                        .viewport = {
                                .position = {0, 0},
                                .size = {76 * g_game.current_level.actual_displayed_health, 3}
                        },
                        .position = {(VIRTUAL_WIDTH / 2) - (82 / 2) + 3, 20 + 2},
                        .scale = {1.0f, 1.0f},
                        .origin = {0, 0},
                        .rotation = 0,
                }
        };
        oct_Draw(&cmd);
    }

    // Draw status effect icons
    const float start_y = 1;
    float x = 1;
    float y = 1;
    int32_t rollover = 0;
    for (int32_t i = 0; i < MAX_ALARMS; i++) {
        if (alarm_is_active(&g_game.player, i)) {
            oct_DrawTexture(g_game.player.alarms[i].icon, (Oct_Vec2){x, y});
            if (rollover == 5) { // TODO: This block isnt working
                y = start_y;
                x += 10;
                rollover = 0;
            } else {
                y += 10;
            }
            rollover += 1;
        }
    }

    // Draw controls -> 1, 135
    const float controls_opacity = 0.5f;
    oct_DrawTextureColour(
            oct_GetAsset(g_game.assets, "hud/controls.png"),
            &(Oct_Colour){.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = controls_opacity},
            (Oct_Vec2){1, 133});

    if (popups_are_active())
        draw_and_update_popups();

}

static TileVisibility tile_visible_to_player(Position tile, Statblock *player_current_stats) {
    const Position player_position = {
            g_game.player.pos[0],
            g_game.player.pos[1]
    };

    int x0 = player_position[0];
    int y0 = player_position[1];
    int x1 = tile[0];
    int y1 = tile[1];

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (x0 != x1 || y0 != y1) {
        if (x0 != player_position[0] || y0 != player_position[1]) {
            TileContents *t = level_get_tile((Position){x0, y0});
            if (t == NULL || t->type == TILE_CONTENTS_TYPE_WALL) {
                const bool remembered = level_tile_is_remembered(tile);
                return (remembered ? TILE_VISIBILITY_PARTIALLY_VISIBLE : TILE_VISIBILITY_NOT_VISIBLE);
            }
        }

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }

    level_set_tile_memory(tile, player_current_stats->learning + (player_current_stats->cartography * 3));
    return TILE_VISIBILITY_FULLY_VISIBLE;
}

static void draw_tiles() {
    float camera_x, camera_y;
    get_camera_coords(&camera_x, &camera_y, nullptr, nullptr);
    const int32_t start_draw_x = (int32_t)floorf((camera_x - CELL_WIDTH) / CELL_WIDTH);
    const int32_t start_draw_y = (int32_t)floorf((camera_y - CELL_HEIGHT) / CELL_HEIGHT);
    const int32_t tile_horizontal = (int32_t)ceilf((GAME_VIEW_WIDTH + (CELL_WIDTH * 2)) / CELL_WIDTH) + 1;
    const int32_t tile_vertical = (int32_t)ceilf((GAME_VIEW_WIDTH + (CELL_WIDTH * 2)) / CELL_WIDTH) + 1;

    set_draw_target(g_game.current_level.level_tex);
    oct_DrawClear(&(Oct_Colour){.r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 0.0f});
    oct_TilemapDrawPart(g_game.current_level.tilemap, start_draw_x, start_draw_y, tile_horizontal, tile_vertical);
    oct_TilemapDrawPart(g_game.current_level.decorations, start_draw_x, start_draw_y, tile_horizontal, tile_vertical);
    reset_draw_target();

    oct_DrawTexture(g_game.current_level.level_tex, (Oct_Vec2){0, 0});

    // Draw the weapons/items on these tiles
    for (int32_t y = start_draw_y; y < start_draw_y + tile_vertical; y++) {
        for (int32_t x = start_draw_x; x < start_draw_x + tile_horizontal; x++) {
            TileContents *tile = level_get_tile((Position){x, y});
            if (tile && tile->extra_contents_type == TILE_EXTRA_CONTENTS_TYPE_WEAPON ||
                tile && tile->extra_contents_type == TILE_EXTRA_CONTENTS_TYPE_ITEM) {
                oct_DrawTexture(oct_GetAsset(g_game.assets, "objects/item.png"), (Oct_Vec2){x * CELL_WIDTH, y * CELL_HEIGHT});
            }
        }
    }
}

static void draw_fog_of_war() {
    // oct_DrawTexture(g_game.current_level.level_tex, (Oct_Vec2){0, 0});
    float camera_x, camera_y;
    get_camera_coords(&camera_x, &camera_y, nullptr, nullptr);
    const int32_t start_draw_x = (int32_t)floorf((camera_x - CELL_WIDTH) / CELL_WIDTH);
    const int32_t start_draw_y = (int32_t)floorf((camera_y - CELL_HEIGHT) / CELL_HEIGHT);
    const int32_t tile_horizontal = (int32_t)ceilf((GAME_VIEW_WIDTH + (CELL_WIDTH * 2)) / CELL_WIDTH) + 1;
    const int32_t tile_vertical = (int32_t)ceilf((GAME_VIEW_WIDTH + (CELL_WIDTH * 2)) / CELL_WIDTH) + 1;
    Statblock player_current_stats;
    character_get_current_stats(&g_game.player, &player_current_stats);
    const Oct_Texture full_block = oct_GetAsset(g_game.assets, "fow.png");
    const Oct_Texture part_block = oct_GetAsset(g_game.assets, "fow_half.png");

    // Draw shadows over places the player can't see
    for (int32_t y = start_draw_y; y < start_draw_y + tile_vertical; y++) {
        for (int32_t x = start_draw_x; x < start_draw_x + tile_horizontal; x++) {
            TileVisibility tile_visibility = tile_visible_to_player((Position){x, y}, &player_current_stats);
            if (tile_visibility != TILE_VISIBILITY_FULLY_VISIBLE) {
                Oct_DrawCommand cmd = {
                        .blendMode = OCT_BLEND_MODE_BLEND,
                        .type = OCT_DRAW_COMMAND_TYPE_TEXTURE,
                        .colour = {.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f},
                        .Texture = {
                                .texture = (tile_visibility == TILE_VISIBILITY_PARTIALLY_VISIBLE ? part_block : full_block),
                                .viewport = {0, 0, OCT_WHOLE_TEXTURE, OCT_WHOLE_TEXTURE},
                                .position = {((float)x * CELL_WIDTH) - 2, ((float)y * CELL_HEIGHT) - 2},
                                .scale = {1, 1},
                                .origin = {0, 0},
                                .rotation = 0,
                        }
                };
                oct_Draw(&cmd);
            }
        }
    }
}

void draw_attack_view() {
    if (g_game.current_level.state != LEVEL_STATE_PLAYER_ATTACK || level_in_attack_animation()) return;
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
    if (contents && (contents->type == TILE_CONTENTS_TYPE_CHARACTER)) {
        if (contents->type == TILE_CONTENTS_TYPE_CHARACTER && contents->character == &g_game.player) return;
        const Traits *target_traits = &contents->character->info.traits;
        Traits attack_traits;
        // Use spell attack traits if the attack view resulted from an attack spell
        if (g_game.current_level.attack_view.spell) {
            assert(g_game.current_level.attack_view.spell->get_traits_callback(&g_game.player, &attack_traits, nullptr));
        } else {
            memcpy(&attack_traits, &g_game.player.weapons[g_game.player.active_weapon].info.traits, sizeof(Traits));
        }
        int32_t dc, pips;
        const AttackFavour favour =  character_get_attack_stats(&g_game.player,
                                                                &attack_traits,
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
                                       oct_GetAsset(g_game.assets, "fnt_dicesmall") :
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

// Draws count pips going in either the left (-1) or right (1) starting at position
static void draw_pips(Oct_Vec2 position, float direction, int32_t count, int32_t initial_count) {
    const float y = position[1];
    const float pip_horizontal_jump = direction * 4;
    const Oct_Asset pip_tex = oct_GetAsset(g_game.assets, "hud/ingamepip.png");
    float x = position[0];
    const int32_t max = count > initial_count ? count : initial_count;
    Oct_Colour bad_colour = {.r = 0.54f, .g = 0.08f, .b = 0.08f, .a = 0.3f};
    Oct_Colour good_colour = {.r = 0.35f, .g = 0.54f, .b = 0.08f, .a = 1.0f};
    Oct_Colour base_colour = {.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f};
    for (int32_t i = 0; i < max; i++) {
        const int32_t pip_index = i + 1;
        Oct_Colour *c = &base_colour;
        if (pip_index > initial_count)
            c = &good_colour;
        else if (pip_index > count)
            c = &bad_colour;
        oct_DrawTextureColour(
            pip_tex,
            c,
            (Oct_Vec2){x, y});
        x += pip_horizontal_jump;
    }
}

// Draws all the skill pips for a skill going down, does not draw the base stat
static void draw_skill_pips_for_group(Oct_Vec2 position, float direction, Statblock *initial_sb, Statblock *sb, BaseStatType group) {
    // TODO: Draw stats that are lower than normal as red and stats higher as green
    const float x = position[0];
    float y = position[1];
    const float pip_move_y = 8;
    for (int32_t i = 0; i < 4; i++) {
        const int32_t *stat = get_skill_pip(sb, group, i);
        const int32_t *initial_stat = get_skill_pip(initial_sb, group, i);
        draw_pips((Oct_Vec2){x, y}, direction, *stat, *initial_stat);
        y += pip_move_y;
    }
}

void draw_level_menu() {
    // We don't bother drawing if the menu has been tweened off-screen
    const float target_y = g_game.current_level.state == LEVEL_STATE_PLAYER_MENU ? 16 : -193;
    g_game.current_level.menu.real_y += (target_y - g_game.current_level.menu.real_y) * 0.4f;
    if (g_game.current_level.menu.real_y < -192) return;

    // Distance between each pip being drawn
    const float start_x = 32;
    const float start_y = g_game.current_level.menu.real_y;

    oct_DrawTexture(oct_GetAsset(g_game.assets, "hud/menubackground.png"), (Oct_Vec2){start_x, start_y});

    if (g_game.current_level.menu.tab == MENU_TAB_STATS)
        oct_DrawTexture(oct_GetAsset(g_game.assets, "hud/menustatspage.png"), (Oct_Vec2){start_x, start_y});

    // Draw the skill pips
    Statblock sb;
    character_get_current_stats(&g_game.player, &sb);
    draw_skill_pips_for_group((Oct_Vec2){start_x +  76, start_y +  57}, +1, &g_game.player.initial_statblock, &sb, BASE_STAT_TYPE_WITS);
    draw_skill_pips_for_group((Oct_Vec2){start_x +  76, start_y + 121}, +1, &g_game.player.initial_statblock, &sb, BASE_STAT_TYPE_MARTIAL);
    draw_skill_pips_for_group((Oct_Vec2){start_x + 165, start_y +  57}, -1, &g_game.player.initial_statblock, &sb, BASE_STAT_TYPE_GRIT);
    draw_skill_pips_for_group((Oct_Vec2){start_x + 165, start_y + 121}, -1, &g_game.player.initial_statblock, &sb, BASE_STAT_TYPE_LEARNING);

    // Draw the DCs for each skill
    Oct_Vec2 text_size;
    Oct_Asset font = oct_GetAsset(g_game.assets, "fnt_pixel");
    const float vertical_offset = 16;
    oct_DrawText(font, (Oct_Vec2){ start_x + 76 + 4, start_y +  57 - vertical_offset}, 1, "- %i", statblock_get_dc(sb.wits));
    oct_DrawText(font, (Oct_Vec2){ start_x + 76 + 4, start_y + 121 - vertical_offset}, 1, "- %i", statblock_get_dc(sb.martial));
    oct_GetTextSize(font, text_size, 1, "%i -", statblock_get_dc(sb.grit));
    oct_DrawText(font, (Oct_Vec2){start_x + 165 - text_size[0], start_y +  57 - vertical_offset}, 1, "%i -", statblock_get_dc(sb.grit));
    oct_GetTextSize(font, text_size, 1, "%i -", statblock_get_dc(sb.learning));
    oct_DrawText(font, (Oct_Vec2){start_x + 165 - text_size[0], start_y + 121 - vertical_offset}, 1, "%i -", statblock_get_dc(sb.learning));
}

void process_character_attack() {
    if (level_attack_animation_complete()) {
        complete_attack_animation();
    }
}

void level_begin() {
    memset(&g_game.current_level, 0, sizeof(Level));

    // Generate level
    LevelGenerationParameters params = {
        .level_size = {40, 40},
        .room_count = {4, 7},
        .room_min_size = {5, 5},
        .room_max_size = {8, 8},
        .extra_hallways = {1, 2},
    };
    Position player_start_pos;
    generate_level(&g_game.current_level, &params, player_start_pos);
    g_game.current_level.tile_visibilities = oct_Zalloc(g_game.allocator, sizeof(int32_t) * params.level_size[0] * params.level_size[1]);
    g_game.current_level.tile_visibilities_turn = oct_Zalloc(g_game.allocator, sizeof(int32_t) * params.level_size[0] * params.level_size[1]);
    player_init(player_start_pos);
    g_game.current_level.menu.real_y = -200;

    // Debug
    for (int32_t i = 0; i < 2; i++) {
        Position slime_spawn;
        level_get_spawn_point(slime_spawn);
        create_slime(level_get_character_slot(), slime_spawn);
    }

    for (int i = 0; i < 5; i++) {
        Position weapon_spawn;
        get_starting_weapon(WEAPON_TYPE_SWORD, &g_game.current_level.weapons[i]);
        level_get_spawn_point(weapon_spawn);
        TileContents *tile = level_get_tile(weapon_spawn);
        tile->extra_contents_type = TILE_EXTRA_CONTENTS_TYPE_WEAPON;
        tile->weapon = &g_game.current_level.weapons[i];
    }

    Position item_spawn = {
            g_game.player.pos[0] + 1,
            g_game.player.pos[1]
    };
    get_small_health_potion(&g_game.current_level.items[5]);
    TileContents *tile = level_get_tile(item_spawn);
    tile->extra_contents_type = TILE_EXTRA_CONTENTS_TYPE_ITEM;
    tile->item = &g_game.current_level.items[5];
    apply_crushed_hand_status_effect(&g_game.player);
    apply_crushed_hand_status_effect(&g_game.player);
    apply_crushed_hand_status_effect(&g_game.player);
    apply_crushed_hand_status_effect(&g_game.player);
    apply_crushed_hand_status_effect(&g_game.player);
    apply_crushed_hand_status_effect(&g_game.player);
    apply_crushed_hand_status_effect(&g_game.player);

    Position item_spawn2 = {
            g_game.player.pos[0] - 1,
            g_game.player.pos[1]
    };
    get_evil_rock(&g_game.current_level.items[6]);
    tile = level_get_tile(item_spawn2);
    tile->extra_contents_type = TILE_EXTRA_CONTENTS_TYPE_ITEM;
    tile->item = &g_game.current_level.items[6];
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
    if (!popups_are_active()) {
        bool next_character = characters_update();
        while (next_character) {
            next_character = characters_update();
        }

        player_update();
    }
    const bool world_turn_occurred = g_game.current_level.world_turn;
    update_camera_coords();

    // Drawing the world
    oct_LockCameras(g_game.world_camera);
    draw_tiles();
    draw_characters();
    draw_fog_of_war();
    draw_attack_view();
    draw_labels();

    // UI drawing
    oct_LockCameras(g_game.ui_camera);
    draw_attack_view_ui();
    draw_ui();
    draw_level_menu();

    process_character_attack();

    timer_tick(&g_game.current_level.Attack.animation_timer);
    if (world_turn_occurred) g_game.current_level.turn++;
    return g_game.level_index;
}

void level_end() {
    oct_Free(g_game.allocator, g_game.current_level.tile_visibilities);
    oct_Free(g_game.allocator, g_game.current_level.tile_visibilities_turn);
    cleanup_level(&g_game.current_level);
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
            g_game.current_level.labels[i].position[0] = ((float)pos[0] * CELL_WIDTH) - ((float)strlen(text) * 6.0f * 0.5f);
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

void level_get_spawn_point(Position out_tile) {
    const int32_t index = random_int(0, g_game.current_level.spawn_points_count);
    out_tile[0] = g_game.current_level.spawn_points[index][0];
    out_tile[1] = g_game.current_level.spawn_points[index][1];

    // Swap the index and last spot to effectively remove an element in O(1)
    const int32_t last_spot = g_game.current_level.spawn_points_count - 1;
    g_game.current_level.spawn_points[index][0] = g_game.current_level.spawn_points[last_spot][0];
    g_game.current_level.spawn_points[index][1] = g_game.current_level.spawn_points[last_spot][1];
    g_game.current_level.spawn_points_count--;
}

int32_t level_get_tile_memory(Position pos) {
    if (pos[0] < 0 || pos[0] >= g_game.current_level.level_width ||
        pos[1] < 0 || pos[1] >= g_game.current_level.level_height) return 0;
    const int32_t raw_value = g_game.current_level.tile_visibilities[(pos[1] * g_game.current_level.level_width) + pos[0]];
    return raw_value - g_game.current_level.turn;
}

void level_set_tile_memory(Position pos, int32_t visibility) {
    if (pos[0] < 0 || pos[0] >= g_game.current_level.level_width ||
        pos[1] < 0 || pos[1] >= g_game.current_level.level_height) return;
    g_game.current_level.tile_visibilities[(pos[1] * g_game.current_level.level_width) + pos[0]] = visibility + g_game.current_level.turn;
    g_game.current_level.tile_visibilities_turn[(pos[1] * g_game.current_level.level_width) + pos[0]] = g_game.current_level.turn;
}

bool level_tile_is_remembered(Position pos) {
    return level_get_tile_memory(pos) > 0;
}

bool level_tile_seen_this_turn(Position pos) {
    if (pos[0] < 0 || pos[0] >= g_game.current_level.level_width ||
        pos[1] < 0 || pos[1] >= g_game.current_level.level_height) return 0;
    const int32_t raw_value = g_game.current_level.tile_visibilities_turn[(pos[1] * g_game.current_level.level_width) + pos[0]];
    return raw_value - g_game.current_level.turn >= -1 || raw_value == g_game.current_level.turn;
}

bool level_extra_player_turn() {
    return g_game.player.cumulative_movement >= 100;
}

void level_set_displayed_enemy(Character *c) {
    if (c != g_game.current_level.enemy_displayed)
        g_game.current_level.actual_displayed_health = (float)c->current_hp / (float)character_max_hp(c);
    g_game.current_level.enemy_displayed = c;
    timer_start(&g_game.current_level.enemy_display_timer, 30 * 3);
}

void level_transition_to_enemy_turns() {
    g_game.current_level.state = LEVEL_STATE_ENEMY_TURN;
    g_game.current_level.world_turn = true;
    g_game.current_level.enemy_turn = -1;
    level_next_enemy_turn();
}

void level_next_enemy_turn() {
    for (int32_t i = g_game.current_level.enemy_turn + 1; i < MAX_CHARACTERS; i++) {
        if (character_is_alive(&g_game.current_level.characters[i])) {
            g_game.current_level.enemy_turn = i;
            return;
        }
    }
    g_game.current_level.state = LEVEL_STATE_PLAYER_INTERACTION;
    g_game.current_level.world_turn = false;
}

void level_extract_tile_item(Position tile, Item *out_item) {
    TileContents *t = level_get_tile(tile);
    assert(t);
    assert(t->extra_contents_type == TILE_EXTRA_CONTENTS_TYPE_ITEM);

    // Move the contents over and remove it from the tile
    memcpy(out_item, t->item, sizeof(Item));
    t->extra_contents_type = TILE_EXTRA_CONTENTS_TYPE_NONE;

    // Remove it from the level list
    for (int32_t i = 0; i < MAX_ITEMS; i++) {
        if (t->item == &g_game.current_level.items[i]) {
            g_game.current_level.items[i].type = ITEM_TYPE_NONE;
            break;
        }
    }
}

void level_extract_tile_weapon(Position tile, Weapon *out_weapon) {
    TileContents *t = level_get_tile(tile);
    assert(t);
    assert(t->extra_contents_type == TILE_EXTRA_CONTENTS_TYPE_WEAPON);

    // Move the contents over and remove it from the tile
    memcpy(out_weapon, t->item, sizeof(Weapon));
    t->extra_contents_type = TILE_EXTRA_CONTENTS_TYPE_NONE;

    // Remove it from the level list
    for (int32_t i = 0; i < MAX_ITEMS; i++) {
        if (t->weapon == &g_game.current_level.weapons[i]) {
            g_game.current_level.weapons[i].type = WEAPON_TYPE_NONE;
            break;
        }
    }
}
