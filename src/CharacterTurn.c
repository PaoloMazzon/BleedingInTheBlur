// This file only deals with processing character turns, as they are massively general
#include <math.h>
#include "Character.h"
#include "Util.h"
#include "Game.h"

static int32_t sign(int32_t x) {
    return x > 0 ? 1 : (x < 0 ? -1 : 0);
}

// Moves a character in a random direction
static void random_movement(Character *c) {
    Position movement_options[] = {
            {1, 0},
            {0, 1},
            {-1, 0},
            {0, -1},
    };
    const int32_t choice = random_int(0, 4);
    Position new_pos = {
            c->pos[0] + movement_options[choice][0],
            c->pos[1] + movement_options[choice][1],
    };
    character_move(c, new_pos);
}

// Returns true if it's possible to attack the player from where the character is
static bool can_attack_player(Character *c, bool line_of_sight) {
    return tile_distance(c->pos, g_game.player.pos) <= c->starting_weapon.range && line_of_sight;
}

// Doesn't check if it's possible
static void attack_player(Character *c) {
    character_attempt_attack(
            c, &c->starting_weapon.info.traits,
            &g_game.player, c->starting_weapon.damage);
}

// Moves diagonally towards a point, with no pathfinding
static void move_towards(Character *c, Position p) {
    const int32_t delta_x = p[0] - c->pos[0];
    const int32_t delta_y = p[1] - c->pos[1];
    if (abs(delta_x) > abs(delta_y)) {
        Position new_spot = {
                c->pos[0] + sign(delta_x),
                c->pos[1]
        };
        character_move(c, new_spot);
    } else {
        Position new_spot = {
                c->pos[0],
                c->pos[1] + sign(delta_y),
        };
        character_move(c, new_spot);
    }
}

void character_take_turn(Character *c) {
    if (!character_is_alive(c)) return;

    // Behaviour-controlling traits
    const bool b_lazy = c->info.traits.Character.lazy;
    const bool b_hostile = c->info.traits.Character.hostile;
    const bool line_of_sight_on_player = !tiles_have_walls_between(c->pos, g_game.player.pos);
    const bool within_aggro_range = tile_distance(c->pos, g_game.player.pos) <= c->aggro_range;

    if (b_hostile && line_of_sight_on_player && within_aggro_range) {
        if (can_attack_player(c, line_of_sight_on_player)) {
            attack_player(c);
        } else {
            move_towards(c, g_game.player.pos);
        }
    } else if (b_lazy) {
        if (oct_Random(0, 1) < 0.25) {
            random_movement(c);
        }
    } else {
        random_movement(c);
    }
}