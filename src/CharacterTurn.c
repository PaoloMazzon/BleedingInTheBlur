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

// Returns the closest character to c that has one of the specified traits, if its not nullptr c can see the target and the target is within aggro range
Character *find_nearest_character_with_trait(Character *c, bool hostile, bool friendly, bool non_player, int32_t aggro_range) {
    Character *found = nullptr;
    int32_t max_distance = INT32_MAX;
    for (int32_t i = 0; i < MAX_CHARACTERS; i++) {
        Character *current = &g_game.current_level.characters[i];
        const int32_t distance = tile_distance(c->pos, current->pos);
        if (character_is_alive(current)
            && distance < max_distance
            && !tiles_have_walls_between(c->pos, current->pos)
            && ((!hostile && !friendly) || (current->info.traits.Character.hostile && hostile) || (current->info.traits.Character.friendly && friendly))
            && ((non_player && current != &g_game.player) || !non_player)
            && distance < aggro_range) {
            max_distance = tile_distance(c->pos, current->pos);
            found = current;
        }
    }
    return found;
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
    const bool is_aggrod_by_character = c->aggro_timer > 0;
    const bool is_sleeping = c->info.traits.Character.sleeping;
    const bool b_berserker = c->info.traits.Character.berserker;
    const bool b_friendly = c->info.traits.Character.friendly;
    const Character *berserk_target = b_berserker ? find_nearest_character_with_trait(c, false, false, false, c->aggro_range) : nullptr;
    const Character *friendly_target = b_friendly ? find_nearest_character_with_trait(c, true, false, true, c->aggro_range) : nullptr;
    const Character *hostile_target = b_hostile ? find_nearest_character_with_trait(c, false, true, false, c->aggro_range) : nullptr;

    // TODO: Process berserker trait (attack nearest character instead of player)
    // TODO: Process friendly trait (attack aggro target first then non-player non-friendly characters)
    // TODO: Process sleeping trait (if player is within a certain range roll player's deception)

    // Attack this character might choose to use
    const int32_t base_attack_damage = c->starting_weapon.damage;
    const int32_t attack_range = c->starting_weapon.range;
    const Traits *attack_traits = &c->starting_weapon.info.traits;

    // This logic train is decently complicated:
    //  1. If the character is currently aggrod by another character, attack the aggro target (ie, another character hit it so they should return the aggression)
    //  2. Berserker enemies will just attack anything they can see.
    //  3. Friendly characters will prioritize hostile characters
    //  4. Hostile creatures will attack friendly creatures they can see
    //  5. Walk around aimlessly otherwise (lazy characters walk around less often)
    if (is_aggrod_by_character) {
        if (b_scared) {
            move_away_from(c, c->aggrod_character->pos);
        } else {
            general_aggression(c, c->aggrod_character, attack_range, attack_traits, base_attack_damage);
        }
    } else if (b_berserker && berserk_target) {
        general_aggression(c, berserk_target, attack_range, attack_traits, base_attack_damage);
    } else if (b_friendly && friendly_target) {
        general_aggression(c, friendly_target, attack_range, attack_traits, base_attack_damage);
    } else if (b_hostile && hostile_target) {
        general_aggression(c, hostile_target, attack_range, attack_traits, base_attack_damage);
    } else if (b_lazy) {
        if (oct_Random(0, 1) < 0.25) {
            random_movement(c);
        }
    } else {
        random_movement(c);
    } // TODO: Friendly creatures should follow the player

    character_process_alarms(c);
}