#include <string.h>
#include "Character.h"
#include "Util.h"

void character_create(Statblock *starting_stats, Character *out) {
    memset(out, 0, sizeof(Character));
    memcpy(&out->initial_statblock, starting_stats, sizeof(Statblock));
    out->current_hp = character_max_hp(out);
    out->current_mana = character_max_mana(out);
}

void character_get_current_stats(Character *c, Statblock *out) {
    memset(out, 0, sizeof(Statblock));
    for (int i = 0; i < BASE_STAT_TYPE_MAX; i++)
        out->base_stats[i] = c->initial_statblock.base_stats[i] + c->bonus_statblock.base_stats[i];
    for (int i = 0; i < GRIT_STAT_TYPE_MAX; i++)
        out->grit_stats[i] = c->initial_statblock.grit_stats[i] + c->bonus_statblock.grit_stats[i];
    for (int i = 0; i < LEARNING_STAT_TYPE_MAX; i++)
        out->learning_stats[i] = c->initial_statblock.learning_stats[i] + c->bonus_statblock.learning_stats[i];
    for (int i = 0; i < WITS_STAT_TYPE_MAX; i++)
        out->wits_stats[i] = c->initial_statblock.wits_stats[i] + c->bonus_statblock.wits_stats[i];
    for (int i = 0; i < MARTIAL_STAT_TYPE_MAX; i++)
        out->martial_stats[i] = c->initial_statblock.martial_stats[i] + c->bonus_statblock.martial_stats[i];
}

int32_t character_max_hp(Character *c) {
    Statblock current_statblock;
    character_get_current_stats(c, &current_statblock);
    return c->bonus_max_hp + (current_statblock.grit * 2) + 15;
}

int32_t character_max_mana(Character *c) {
    Statblock current_statblock;
    character_get_current_stats(c, &current_statblock);
    return c->bonus_max_mana + (current_statblock.learning * 2);
}

int32_t character_movement(Character *c) {
    Statblock current_statblock;
    character_get_current_stats(c, &current_statblock);
    return c->bonus_movement + current_statblock.wits;
}

int32_t character_crit_chance(Character *c) {
    Statblock current_statblock;
    character_get_current_stats(c, &current_statblock);
    return c->bonus_crit_chance + current_statblock.martial;
}

int32_t character_take_damage(Character *c, int32_t damage, Traits *source_traits) {
    const int32_t initial = c->current_hp;
    c->current_hp = non_negative(c->current_hp - damage);
    return initial - c->current_hp;
}

bool character_alive(Character *c) {
    return c->current_hp > 0 || c->info.traits.undying;
}
