#include "Game.h"
#include "Character.h"
#include "Util.h"

void player_init() {
    Statblock sb;
    random_statblock(&sb);
    print_statblock(&sb);
    character_create(&sb, (Position){10, 10}, &g_game.player);
    g_game.player.info.drawn_type = DRAWN_TYPE_TEXTURE;
    g_game.player.info.texture = oct_GetAsset(g_game.assets, "characters/player.png");
    get_starting_weapon(WEAPON_TYPE_SPEAR, &g_game.player.starting_weapon);
}

void player_update() {
    Character *player = &g_game.player;

    // Should only be 1/-1 in both directions
    Position movement_direction = {0};
    bool player_has_taken_actions = false;
    if (g_game.current_level.state == LEVEL_STATE_PLAYER_ATTACK) {
        if (oct_KeyPressed(BUTTON_LEFT)) movement_direction[0] = -1;
        else if (oct_KeyPressed(BUTTON_RIGHT)) movement_direction[0] = 1;
        else if (oct_KeyPressed(BUTTON_UP)) movement_direction[1] = -1;
        else if (oct_KeyPressed(BUTTON_DOWN)) movement_direction[1] = 1;

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
        if (oct_KeyPressed(BUTTON_ATTACK_VIEW)) {
            g_game.current_level.state = LEVEL_STATE_PLAYER_INTERACTION;
        }

        // Attack things at target
        if (oct_KeyPressed(BUTTON_CONFIRM)) {
            const Traits *tile_traits = level_get_tile_traits(g_game.current_level.attack_view.attack_cursor);
            if (tile_traits && tile_distance(g_game.current_level.attack_view.attack_cursor, player->pos) <= player->weapons[player->active_weapon].range) {
                int32_t bonus_damage = 0;
                bool attack = character_attempt_attack(player,
                                                       &player->weapons[player->active_weapon].info.traits,
                                                       g_game.current_level.attack_view.attack_cursor,
                                                       tile_traits,
                                                       &bonus_damage);
                // TODO: Attack damage and animation
            }
        }

        // Look at attack cursor
        look_at(g_game.current_level.attack_view.attack_cursor, 0.8f);
    } else if (g_game.current_level.state == LEVEL_STATE_PLAYER_INTERACTION) {
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

        // Process movement
        if (movement_direction[0] != 0 || movement_direction[1] != 0) {
            const Position target_position = {
                    g_game.player.pos[0] + movement_direction[0],
                    g_game.player.pos[1] + movement_direction[1],
            };
            if (character_move(&g_game.player, target_position))
                player_has_taken_actions = true;
        }

        // Point camera at player
        look_at(player->pos, 1);
    }

    // Player can toggle the stat view anytime
    if (oct_KeyPressed(BUTTON_STAT_TOGGLE)) {
        g_game.current_level.stats_toggle = !g_game.current_level.stats_toggle;
    }

    // State machine type stuff. The actual actions are taken above but this is to maintain proper
    // usage of the state machine and also extra turns.
    if (player_has_taken_actions) {
        // World gets to take a turn unless player has accumulated enough movement to get
        // an extra turn
        if (g_game.player.cumulative_movement == 100) {
            g_game.player.cumulative_movement = 0;
        } else {
            g_game.current_level.world_turn = true;
            g_game.current_level.state = LEVEL_STATE_ENEMY_TURN;
        }
        g_game.player.cumulative_movement += oct_Clampi(0, 100, character_movement(&g_game.player));
    }
}
