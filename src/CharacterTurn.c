// This file only deals with processing character turns, as they are massively general
#include <math.h>
#include <assert.h>
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
static bool can_attack_character(Character *c, Character *other, bool line_of_sight, int32_t weapon_range) {
    return tile_distance(c->pos, other->pos) <= weapon_range && line_of_sight;
}

// Doesn't check if it's possible
static void attack_character(Character *c, Character *other, Traits *traits, int32_t base_damage) {
    character_attempt_attack(
            c, traits,
            other, base_damage);
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
        if (!character_move(c, new_spot)) {
            character_move(c, (Position) {c->pos[0], c->pos[1] + sign(delta_y)});
        }
    } else {
        Position new_spot = {
                c->pos[0],
                c->pos[1] + sign(delta_y),
        };
        if (!character_move(c, new_spot)) {
            character_move(c, (Position) {c->pos[0] + sign(delta_x), c->pos[1]});
        }
    }
}

// Moves diagonally away from a point, with no pathfinding
static void move_away_from(Character *c, Position p) {
    const int32_t delta_x = p[0] - c->pos[0];
    const int32_t delta_y = p[1] - c->pos[1];
    if (abs(delta_x) > abs(delta_y)) {
        Position new_spot = {
                c->pos[0] - sign(delta_x),
                c->pos[1]
        };
        character_move(c, new_spot);
    } else {
        Position new_spot = {
                c->pos[0],
                c->pos[1] - sign(delta_y),
        };
        character_move(c, new_spot);
    }
}

// Top-level function for a character to show aggression to another; ie move towards them and attack once its possible to
static void general_aggression(Character *c, Character *other, int32_t range, Traits *damage_traits, int32_t base_damage) {
    const bool line_of_sight = !tiles_have_walls_between(c->pos, other->pos) || !tiles_have_walls_between(other, c->pos);
    if (can_attack_character(c, other, line_of_sight, range)) {
        attack_character(c, other, damage_traits, base_damage);
    } else {
        move_towards(c, other->pos);
    }
}

void character_take_turn(Character *c) {
    if (!character_is_alive(c)) return;

    // If their previous aggressor has passed remove the status
    if (c->aggrod_character && (!character_is_alive(c->aggrod_character) || c->aggro_timer == 0)) {
        c->aggrod_character = nullptr;
        c->aggro_timer = 0;
    }
    assert((c->aggro_timer > 0 && c->aggrod_character) || (c->aggro_timer == 0 && !c->aggrod_character));

    // Behaviour-controlling traits
    const bool b_lazy = c->info.traits.Character.lazy;
    const bool b_hostile = c->info.traits.Character.hostile;
    const bool b_scared = c->info.traits.Character.scared;
    const bool line_of_sight_on_player = !tiles_have_walls_between(c->pos, g_game.player.pos) || !tiles_have_walls_between(g_game.player.pos, c->pos);
    const bool within_aggro_range = tile_distance(c->pos, g_game.player.pos) <= c->aggro_range;
    const bool is_aggrod_by_character = c->aggro_timer > 0;

    // Attack this character might choose to use
    const int32_t base_attack_damage = c->starting_weapon.damage;
    const int32_t attack_range = c->starting_weapon.range;
    const Traits *attack_traits = &c->starting_weapon.info.traits;

    if (is_aggrod_by_character) {
        if (b_scared) {
            move_away_from(c, c->aggrod_character->pos);
        } else {
            general_aggression(c, c->aggrod_character, attack_range, attack_traits, base_attack_damage);
        }
    } else if (b_hostile && line_of_sight_on_player && within_aggro_range) {
        general_aggression(c, &g_game.player, attack_range, attack_traits, base_attack_damage);
    } else if (b_lazy) {
        if (oct_Random(0, 1) < 0.25) {
            random_movement(c);
        }
    } else {
        random_movement(c);
    }
}