#include <oct/Octarine.h>
#include "Game.h"
#include "Character.h"

void player_update() {
    // Should only be 1/-1 in both directions
    Position movement_direction = {0};
    if (oct_KeyPressed(OCT_KEY_LEFT)) movement_direction[0] = -1;
    else if (oct_KeyPressed(OCT_KEY_RIGHT)) movement_direction[0] = 1;
    else if (oct_KeyPressed(OCT_KEY_UP)) movement_direction[1] = -1;
    else if (oct_KeyPressed(OCT_KEY_DOWN)) movement_direction[1] = 1;

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
        c_info->actual_position[0] += (c_info->target_position[0] - c_info->actual_position[0]) * 0.2f;
        c_info->actual_position[1] += (c_info->target_position[1] - c_info->actual_position[1]) * 0.2f;

        character_draw(c, c_info->actual_position);
    }
}

void draw_ui() {
    // TODO: This
}

void level_begin() {
    Statblock sb;
    random_statblock(&sb);
    print_statblock(&sb);
    character_create(&sb, &g_game.player);
    g_game.player.info.drawn_type = DRAWN_TYPE_TEXTURE;
    g_game.player.info.texture = oct_GetAsset(g_game.assets, "player.png");
}

LevelIndex level_update() {
    player_update();
    characters_update();
    draw_characters();
    draw_ui();

    return g_game.level_index;
}

void level_end() {

}
