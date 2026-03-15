#include <assert.h>
#include <string.h>
#include "Game.h"
#include "Character.h"
#include "Util.h"
#include "WeaponItem.h"

void player_init(Position start_pos) {
    Statblock sb;
    random_statblock(&sb);
    print_statblock(&sb);
    character_create(&sb, (Position){start_pos[0], start_pos[1]}, &g_game.player);
    g_game.player.info.drawn_type = DRAWN_TYPE_TEXTURE;
    g_game.player.info.texture = oct_GetAsset(g_game.assets, "characters/player.png");
    get_starting_weapon(WEAPON_TYPE_SPEAR, &g_game.player.starting_weapon);
}

static bool player_attack_view_state() {
    Character *player = &g_game.player;
    Position movement_direction = {0};

    if (level_attack_animation_complete()) {
        return true;
    }

    if (!level_in_attack_animation()) {
        if (oct_KeyPressed(BUTTON_LEFT)) movement_direction[0] = -1;
        else if (oct_KeyPressed(BUTTON_RIGHT)) movement_direction[0] = 1;
        else if (oct_KeyPressed(BUTTON_UP)) movement_direction[1] = -1;
        else if (oct_KeyPressed(BUTTON_DOWN)) movement_direction[1] = 1;
    }

    g_game.current_level.attack_view.attack_cursor[0] += movement_direction[0];
    g_game.current_level.attack_view.attack_cursor[1] += movement_direction[1];
    g_game.current_level.attack_view.attack_cursor[0] = oct_Clampi(
            player->pos[0] - 5,
            player->pos[0] + 5,
            g_game.current_level.attack_view.attack_cursor[0]);
    g_game.current_level.attack_view.attack_cursor[1] = oct_Clampi(
            player->pos[1] - 5,
            player->pos[1] + 5,
            g_game.current_level.attack_view.attack_cursor[1]);

    // Let player cancel attack selection
    if (!level_in_attack_animation() && oct_KeyPressed(BUTTON_ATTACK_VIEW)) {
        g_game.current_level.state = LEVEL_STATE_PLAYER_INTERACTION;
    }

    // Attack things at target
    if (!level_in_attack_animation() && oct_KeyPressed(BUTTON_CONFIRM)) {
        const TileContents *tile = level_get_tile(g_game.current_level.attack_view.attack_cursor);
        if (tile && tile->type == TILE_CONTENTS_TYPE_CHARACTER && tile_distance(g_game.current_level.attack_view.attack_cursor, player->pos) <= player->weapons[player->active_weapon].range) {
            character_attempt_attack(player,
                                     &player->weapons[player->active_weapon].info.traits,
                                     tile->character,
                                     player->weapons[player->active_weapon].damage);
        }
    }

    // Focus on enemies the player is looking directly at
    const TileContents *tile = level_get_tile(g_game.current_level.attack_view.attack_cursor);
    if (tile && tile->type == TILE_CONTENTS_TYPE_CHARACTER && tile_distance(g_game.current_level.attack_view.attack_cursor, player->pos) <= player->weapons[player->active_weapon].range && &g_game.player != tile->character) {
        level_set_displayed_enemy(tile->character);
    }

    // Look at attack cursor
    look_at(g_game.current_level.attack_view.attack_cursor, 0.8f);
    return false;
}

static bool player_interaction_state() {
    Character *player = &g_game.player;
    int32_t item_index;
    bool selected_the_weapon;

    // Skip the turn if a popup just resolved
    if (popup_get_weapon(g_game.current_level.weapon_popup, &selected_the_weapon)) {
        debug("Player completed weapon popup with result %s.", selected_the_weapon ? "true" : "false");
        if (selected_the_weapon) {
            level_extract_tile_weapon(g_game.player.pos, &g_game.player.soul_bound_weapon);
        }
        return true;
    }
    if (popup_get_item(g_game.current_level.item_popup, &item_index)) {
        debug("Player completed item popup with result %i.", item_index);
        if (item_index != -1) {
            // Call the old item's exit inventory callback
            if (g_game.player.items[item_index].exit_inventory_callback)
                g_game.player.items[item_index].exit_inventory_callback(&g_game.player);
            level_extract_tile_item(g_game.player.pos, &g_game.player.items[item_index]);
            // Call the new item's enter inventory callback
            if (g_game.player.items[item_index].enter_inventory_callback)
                g_game.player.items[item_index].enter_inventory_callback(&g_game.player);
        }
        return true;
    }

    bool player_has_taken_actions = false;
    Position movement_direction = {0};
    if (oct_KeyPressed(BUTTON_LEFT)) movement_direction[0] = -1;
    else if (oct_KeyPressed(BUTTON_RIGHT)) movement_direction[0] = 1;
    else if (oct_KeyPressed(BUTTON_UP)) movement_direction[1] = -1;
    else if (oct_KeyPressed(BUTTON_DOWN)) movement_direction[1] = 1;

    if (oct_KeyPressed(BUTTON_ATTACK_VIEW)) {
        g_game.current_level.state = LEVEL_STATE_PLAYER_ATTACK;
        g_game.current_level.attack_view.attack_cursor[0] = player->pos[0] + (int32_t)player->info.facing_direction;
        g_game.current_level.attack_view.attack_cursor[1] = player->pos[1];
        g_game.current_level.attack_view.cursor_real_pos[0] = (float)g_game.current_level.attack_view.attack_cursor[0] * CELL_WIDTH;
        g_game.current_level.attack_view.cursor_real_pos[1] = (float)g_game.current_level.attack_view.attack_cursor[1] * CELL_HEIGHT;
    }

    if (oct_KeyPressed(BUTTON_SWAP_WEAPON)) {
        player->active_weapon = !player->active_weapon;
    }

    if (oct_KeyPressed(BUTTON_ITEM_SWAP)) {
        timer_start(&g_game.current_level.player_item_bar_popup_timer, 30 * 3);
        g_game.player.selected_item = (g_game.player.selected_item + 1) % INVENTORY_SIZE;
    }

    if (oct_KeyPressed(BUTTON_ITEM_USE) && player->items[player->selected_item].type != ITEM_TYPE_NONE) {
        if (!use_item(&player->items[player->selected_item], player)) {
            player->items[player->selected_item].type = ITEM_TYPE_NONE;
        }
    }

    // Process movement
    if (movement_direction[0] != 0 || movement_direction[1] != 0) {
        const Position target_position = {
                g_game.player.pos[0] + movement_direction[0],
                g_game.player.pos[1] + movement_direction[1],
        };
        TileContents *t = level_get_tile(target_position);
        if (character_move(&g_game.player, target_position)) {
            // If we moved onto a weapon/item, we should show the popup to pick it up before passing turn
            TileContents *tile = level_get_tile(player->pos);
            if (tile->extra_contents_type == TILE_EXTRA_CONTENTS_TYPE_WEAPON) {
                g_game.current_level.weapon_popup = popup_weapon_select(tile->weapon);
                debug("Player hit weapon popup.");
            } else if (tile->extra_contents_type == TILE_EXTRA_CONTENTS_TYPE_ITEM) {
                g_game.current_level.item_popup = popup_item_select(tile->item);
                debug("Player hit item popup.");
            } else {
                player_has_taken_actions = true;
            }
        } else if (t && t->type == TILE_CONTENTS_TYPE_CHARACTER && tile_distance(target_position, player->pos) <= player->weapons[player->active_weapon].range) {
            // Auto-attack characters by moving into them
            character_attempt_attack(player,
                                     &player->weapons[player->active_weapon].info.traits,
                                     t->character,
                                     player->weapons[player->active_weapon].damage);
            level_set_displayed_enemy(t->character);
            g_game.current_level.state = LEVEL_STATE_PLAYER_ATTACK;
        }
    }

    // Point camera at player
    look_at(player->pos, 1);

    return player_has_taken_actions;
}

void player_update() {
    Character *player = &g_game.player;

    // Should only be 1/-1 in both directions
    bool player_has_taken_actions = false;
    if (g_game.current_level.state == LEVEL_STATE_PLAYER_ATTACK) {
        player_has_taken_actions = player_attack_view_state();
    } else if (g_game.current_level.state == LEVEL_STATE_PLAYER_INTERACTION) {
        player_has_taken_actions = player_interaction_state();
    }

    // Player can toggle the stat view anytime
    if (oct_KeyPressed(BUTTON_STATUS_TOGGLE)) {
        g_game.current_level.stats_toggle = !g_game.current_level.stats_toggle;
    }

    // State machine type stuff. The actual actions are taken above but this is to maintain proper
    // usage of the state machine and also extra turns.
    if (player_has_taken_actions) {
        // World gets to take a turn unless player has accumulated enough movement to get
        // an extra turn
        if (g_game.player.cumulative_movement == 100) {
            g_game.player.cumulative_movement = 0;
            g_game.current_level.state = LEVEL_STATE_PLAYER_INTERACTION;
        } else {
            level_transition_to_enemy_turns();
        }
        g_game.player.cumulative_movement = oct_Clampi(0, 100, g_game.player.cumulative_movement + character_movement(&g_game.player));
    }
}
