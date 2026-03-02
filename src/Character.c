#include <string.h>
#include <assert.h>
#include "Character.h"
#include "Util.h"

void get_starting_weapon(WeaponType weapon_type, Weapon *out) {
    memset(out, 0, sizeof(Weapon));
    if (weapon_type == WEAPON_TYPE_SWORD) {
        get_weapon(weapon_type, RARITY_COMMON, out);
    } else if (weapon_type == WEAPON_TYPE_SPEAR) {
        get_weapon(weapon_type, RARITY_COMMON, out);
    } else if (weapon_type == WEAPON_TYPE_BOW) {
        get_weapon(weapon_type, RARITY_COMMON, out);
    } else if (weapon_type == WEAPON_TYPE_CROSSBOW) {
        get_weapon(weapon_type, RARITY_COMMON, out);
    } else if (weapon_type == WEAPON_TYPE_DAGGER) {
        get_weapon(weapon_type, RARITY_COMMON, out);
    } else {
        // This will only trip if a weapon hasn't been implemented
        assert(false);
    }
}



void get_weapon(WeaponType weapon_type, Rarity rarity, Weapon *out) {
    memset(out, 0, sizeof(Weapon));
    // TODO: Do something with rarity
    if (weapon_type == WEAPON_TYPE_SWORD) {
        out->info.name = "Sword";
        out->damage = 5;
        out->range = 1;
        out->bonus_stats.attrition = 1;
    } else if (weapon_type == WEAPON_TYPE_SPEAR) {
        out->info.name = "Spear";
        out->damage = 4;
        out->range = 2;
        out->bonus_stats.marksman = 1;
    } else if (weapon_type == WEAPON_TYPE_BOW) {
        out->info.name = "Bow";
        out->damage = 2;
        out->range = 4;
    } else if (weapon_type == WEAPON_TYPE_CROSSBOW) {
        out->info.name = "Crossbow";
        out->damage = 3;
        out->range = 4;
        out->bonus_stats.evade = -1;
        out->bonus_stats.marksman = 1;
    } else if (weapon_type == WEAPON_TYPE_DAGGER) {
        out->info.name = "Dagger";
        out->damage = 2;
        out->range = 1;
        out->bonus_stats.evade = 1;
    } else {
        // This will only trip if a weapon hasn't been implemented
        assert(false);
    }
}

void random_statblock(Statblock *out) {
    memset(out, 0, sizeof(Statblock));

    // Pick base stats first
    for (int i = 0; i < STARTING_BASE_STAT_POINTS;) {
        const int32_t stat = random_int(0, 4);
        if (out->base_stats[stat] >= BASE_STAT_MAX) continue;
        out->base_stats[stat] += 1;
        i++;
    }

    // Pick skill pips
    for (int i = 0; i < STARTING_SKILL_PIPS; i++) {
        const int32_t base_stat = random_int(0, 4);
        const int32_t skill = random_int(0, 4);
        int32_t *skill_stat = get_skill_pip(out, base_stat, skill);
        *skill_stat += 1;
    }
}

void print_statblock(Statblock *s) {
    char skill_score[26] = "=========================";
    char skill_name_offset[13] = "            ";
    for (int base_stat = 0; base_stat < 4; base_stat++) {
        oct_Log("---------- %s: %i", BASE_STAT_NAMES[base_stat], s->base_stats[base_stat]);
        for (int skill = 0; skill < 4; skill++) {
            const int32_t skill_pips = *get_skill_pip(s, base_stat, skill);
            const char *skill_name = get_skill_name(base_stat, skill);
            skill_score[skill_pips] = '\0';
            skill_name_offset[12 - strlen(skill_name)] = '\0'; // might cause problems on a rename
            // If this fails that means a name was changed without updating the skill_name_offset size
            assert(strlen(skill_name) <= 12);
            oct_Log("%s%s |%s", skill_name_offset, skill_name, skill_score);
            skill_score[skill_pips] = '=';
            skill_name_offset[12 - strlen(skill_name)] = ' ';
        }
    }
    oct_Log("----------");
}

int32_t *get_skill_pip(Statblock *s, int32_t base_stat_index, int32_t skill_index) {
    if (base_stat_index == BASE_STAT_TYPE_GRIT) {
        return &s->grit_stats[skill_index];
    } else if (base_stat_index == BASE_STAT_TYPE_LEARNING) {
        return &s->learning_stats[skill_index];
    } else if (base_stat_index == BASE_STAT_TYPE_WITS) {
        return &s->wits_stats[skill_index];
    }
    return &s->martial_stats[skill_index];
}

const char *get_skill_name(int32_t base_stat_index, int32_t skill_index) {
    if (base_stat_index == BASE_STAT_TYPE_GRIT) {
        return GRIT_STAT_NAMES[skill_index];
    } else if (base_stat_index == BASE_STAT_TYPE_LEARNING) {
        return LEARNING_STAT_NAMES[skill_index];
    } else if (base_stat_index == BASE_STAT_TYPE_WITS) {
        return WITS_STAT_NAMES[skill_index];
    }
    return MARTIAL_STAT_NAMES[skill_index];
}

void character_draw(Character *c, Oct_Vec2 position) {
    if (c->info.drawn_type == DRAWN_TYPE_SPRITE) {
        oct_DrawSpriteInt(
                OCT_INTERPOLATE_ALL, c->info.id,
                c->info.sprite, &c->info.sprite_instance,
                position);
    } else if (c->info.drawn_type == DRAWN_TYPE_TEXTURE) {
        oct_DrawTextureInt(
                OCT_INTERPOLATE_ALL, c->info.id,
                c->info.texture,
                position);
    }
}

void character_create(Statblock *starting_stats, Character *out) {
    memset(out, 0, sizeof(Character));
    memcpy(&out->initial_statblock, starting_stats, sizeof(Statblock));
    out->info.id = new_oct_id();
    out->current_hp = character_max_hp(out);
    out->current_mana = character_max_mana(out);
}

void character_get_current_stats(Character *c, Statblock *out) {
    memset(out, 0, sizeof(Statblock));
    for (int i = 0; i < BASE_STAT_TYPE_MAX; i++)
        out->base_stats[i] = non_negative(c->initial_statblock.base_stats[i] +
        c->bonus_statblock.base_stats[i] +
        c->weapons[c->active_weapon].bonus_stats.base_stats[i]);
    for (int i = 0; i < GRIT_STAT_TYPE_MAX; i++)
        out->grit_stats[i] = non_negative(c->initial_statblock.grit_stats[i] +
        c->bonus_statblock.grit_stats[i] +
        c->weapons[c->active_weapon].bonus_stats.grit_stats[i]);
    for (int i = 0; i < LEARNING_STAT_TYPE_MAX; i++)
        out->learning_stats[i] = non_negative(c->initial_statblock.learning_stats[i] +
        c->bonus_statblock.learning_stats[i] +
        c->weapons[c->active_weapon].bonus_stats.learning_stats[i]);
    for (int i = 0; i < WITS_STAT_TYPE_MAX; i++)
        out->wits_stats[i] = non_negative(c->initial_statblock.wits_stats[i] +
        c->bonus_statblock.wits_stats[i] +
        c->weapons[c->active_weapon].bonus_stats.wits_stats[i]);
    for (int i = 0; i < MARTIAL_STAT_TYPE_MAX; i++)
        out->martial_stats[i] = non_negative(c->initial_statblock.martial_stats[i] +
        c->bonus_statblock.martial_stats[i] +
        c->weapons[c->active_weapon].bonus_stats.martial_stats[i]);
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

bool character_is_alive(Character *c) {
    return c->current_hp > 0 || c->info.traits.undying;
}

int32_t character_evade_pips(Character *c) {
    Statblock current_statblock;
    character_get_current_stats(c, &current_statblock);
    return current_statblock.evade;
}

bool character_move(Character *c, const Position new_position) {
    if (c->status_effects.grappled > 0) return false;
    c->pos[0] = new_position[0];
    c->pos[1] = new_position[1];
    c->info.target_position[0] = (float)new_position[0] * CELL_WIDTH;
    c->info.target_position[1] = (float)new_position[1] * CELL_HEIGHT;
    return true;
}
