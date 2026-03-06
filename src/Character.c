#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "Character.h"
#include "Util.h"
#include "Game.h"

void get_starting_weapon(WeaponType weapon_type, Weapon *out) {
    memset(out, 0, sizeof(Weapon));
    get_weapon(weapon_type, RARITY_COMMON, out);
}

void get_weapon(WeaponType weapon_type, Rarity rarity, Weapon *out) {
    memset(out, 0, sizeof(Weapon));
    // TODO: Do something with rarity
    if (weapon_type == WEAPON_TYPE_SWORD) {
        out->info.name = "Sword";
        out->damage = 5;
        out->range = 1;
        out->bonus_stats.attrition = 1;
        out->info.traits.Attack.melee = true;
        out->info.traits.Attack.blade = true;
    } else if (weapon_type == WEAPON_TYPE_SPEAR) {
        out->info.name = "Spear";
        out->damage = 4;
        out->range = 2;
        out->bonus_stats.marksman = 1;
        out->info.traits.Attack.melee = true;
        out->info.traits.Attack.heavy = true;
    } else if (weapon_type == WEAPON_TYPE_BOW) {
        out->info.name = "Bow";
        out->damage = 2;
        out->range = 4;
        out->info.traits.Attack.ranged = true;
    } else if (weapon_type == WEAPON_TYPE_CROSSBOW) {
        out->info.name = "Crossbow";
        out->damage = 3;
        out->range = 4;
        out->bonus_stats.evade = -1;
        out->bonus_stats.marksman = 1;
        out->info.traits.Attack.ranged = true;
        out->info.traits.Attack.heavy = true;
    } else if (weapon_type == WEAPON_TYPE_DAGGER) {
        out->info.name = "Dagger";
        out->damage = 2;
        out->range = 1;
        out->bonus_stats.evade = 1;
        out->info.traits.Attack.improvised = true;
        out->info.traits.Attack.blade = true;
        out->info.traits.Attack.melee = true;
    } else if (weapon_type == WEAPON_TYPE_OTHER) {
        out->info.name = "Claw";
        out->damage = 2;
        out->range = 1;
        out->info.traits.Attack.improvised = true;
        out->info.traits.Attack.melee = true;
    } else {
        oct_Raise(OCT_STATUS_ERROR, true, "Weapon type %i hasn't been implemented.", weapon_type);
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
    const float x_offset = 0;c->info.scale_x < 0 ? CELL_WIDTH * -c->info.scale_x : 0;
    if (c->info.drawn_type == DRAWN_TYPE_SPRITE) {
        oct_DrawSpriteIntExt(
                OCT_INTERPOLATE_POSITION | OCT_INTERPOLATE_ROTATION, c->info.id,
                c->info.sprite, &c->info.sprite_instance,
                (Oct_Vec2){position[0] + 4, position[1] + 4},
                (Oct_Vec2){c->info.scale_x, 1},
                c->info.rotation, (Oct_Vec2){OCT_ORIGIN_MIDDLE, OCT_ORIGIN_MIDDLE});
    } else if (c->info.drawn_type == DRAWN_TYPE_TEXTURE) {
        oct_DrawTextureIntExt(
                OCT_INTERPOLATE_POSITION | OCT_INTERPOLATE_ROTATION, c->info.id,
                c->info.texture,
                (Oct_Vec2){position[0] + 4, position[1] + 4},
                (Oct_Vec2){c->info.scale_x, 1},
                c->info.rotation, (Oct_Vec2){OCT_ORIGIN_MIDDLE, OCT_ORIGIN_MIDDLE});
    }
}

void character_create(Statblock *starting_stats, Position position, Character *out) {
    memset(out, 0, sizeof(Character));
    memcpy(&out->initial_statblock, starting_stats, sizeof(Statblock));
    out->info.id = new_oct_id();
    out->current_hp = character_max_hp(out);
    out->current_mana = character_max_mana(out);
    out->info.scale_x = 1;
    out->info.facing_direction = 1;
    out->pos[0] = position[0];
    out->pos[1] = position[1];
    out->info.actual_position[0] = (float)position[0] * CELL_WIDTH;
    out->info.actual_position[1] = (float)position[1] * CELL_HEIGHT;
    out->info.target_position[0] = (float)position[0] * CELL_WIDTH;
    out->info.target_position[1] = (float)position[1] * CELL_HEIGHT;

    // Place character into their proper tile, character_move will take it from there
    TileContents *tile = level_get_tile(position);
    if (tile && tile->type == TILE_CONTENTS_TYPE_NONE) {
        tile->type = TILE_CONTENTS_TYPE_CHARACTER;
        tile->character = out;
    } else {
        oct_Raise(OCT_STATUS_ERROR, true, "Character was attempted to be placed out of bounds.");
    }
}

int32_t statblock_get_dc(int32_t base_stat) {
    return 20 - base_stat;
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
    return c->current_hp > 0 || c->info.traits.Character.undying;
}

int32_t character_evade_pips(Character *c) {
    Statblock current_statblock;
    character_get_current_stats(c, &current_statblock);
    return current_statblock.evade;
}

bool character_move(Character *c, const Position new_position) {
    // Characters can't move if they're grappled
    if (c->status_effects.grappled > 0) {
        create_label("Grappled!", c->pos, (Oct_Colour){.r = 0.9f, .g = 0.1f, .b = 0.05f, .a = 1.0f}, false);
        return false;
    }

    // Make sure we aren't walking into a wall or character
    TileContents *next_tile = level_get_tile(new_position);
    if (next_tile && (next_tile->type == TILE_CONTENTS_TYPE_WALL || next_tile->type == TILE_CONTENTS_TYPE_CHARACTER))
        return false;

    // Remove them from their current tile
    TileContents *tile = level_get_tile(c->pos);
    if (tile && tile->type == TILE_CONTENTS_TYPE_CHARACTER && tile->character == c) {
        tile->type = TILE_CONTENTS_TYPE_NONE;
        tile->character = nullptr;
    }

    c->pos[0] = new_position[0];
    c->pos[1] = new_position[1];
    c->info.target_position[0] = (float)new_position[0] * CELL_WIDTH;
    c->info.target_position[1] = (float)new_position[1] * CELL_HEIGHT;

    // Place character in new tile
    if (next_tile) {
        next_tile->type = TILE_CONTENTS_TYPE_CHARACTER;
        next_tile->character = c;
    }

    return true;
}

AttackFavour character_get_attack_stats(Character *c, const Traits *attack_traits, Position target_position, const Traits *target_traits, int32_t *out_pips, int32_t *out_dc) {
    Statblock current;
    character_get_current_stats(c, &current);
    int32_t pip_count = 0;
    int32_t dc = statblock_get_dc(current.martial);

    // Calculate base pips and dc for the situation
    if (attack_traits->Attack.melee)
        pip_count = current.blades;
    else if (attack_traits->Attack.ranged)
        pip_count = current.marksman;
    else
        pip_count = current.grappler;
    const int32_t base_pips = pip_count;

    // Bonuses and detriments
    if (target_traits->Character.nimble && attack_traits->Attack.ranged) pip_count -= 1;
    if (target_traits->Character.lazy && attack_traits->Attack.heavy) pip_count += 1;
    if (target_traits->occult && attack_traits->holy) pip_count += 1;
    if (target_traits->holy && attack_traits->occult) pip_count += 1;
    if (tile_distance(target_position, c->pos) == 1 && attack_traits->Attack.ranged) pip_count -= 1;

    // Calculate if this roll is favoured, ill favoured, or neutral
    if (out_dc) *out_dc = dc;
    if (out_pips) *out_pips = pip_count;
    if (pip_count > base_pips)
        return ATTACK_FAVOUR_GOOD;
    else if (pip_count < base_pips)
        return ATTACK_FAVOUR_BAD;
    return ATTACK_FAVOUR_NEUTRAL;
}

bool character_attempt_attack(Character *c, const Traits *attack_traits, Character *rcvr, int32_t base_attack_damage) {
    // Get attempt results
    int32_t pips, dc, result;
    const Position target_position = {rcvr->pos[0], rcvr->pos[1]};
    const Traits *target_traits = &rcvr->info.traits;
    character_get_attack_stats(c, attack_traits, target_position, target_traits, &pips, &dc);
    bool passed = roll_dice(pips, dc, &result);
    const int32_t bonus_damage = passed ? result - dc : 0;

    // Setup the level attack animation
    g_game.current_level.Attack.damage = bonus_damage + base_attack_damage;
    g_game.current_level.Attack.successful = passed;
    timer_start(&g_game.current_level.Attack.animation_timer, ATTACK_ANIMATION_DURATION);
    g_game.current_level.Attack.tex = oct_GetAsset(g_game.assets, "attacks/spear.png");
    g_game.current_level.Attack.ranged = attack_traits->Attack.ranged;
    g_game.current_level.Attack.attacker = c;
    g_game.current_level.Attack.receiver = rcvr;

    // Make a dice label
    const Oct_Colour red = {
            .r = 1.0f,
            .g = 0.2f,
            .b = 0.2f,
            .a = 1.0f
    };
    const Oct_Colour green = {
            .r = 0.2f,
            .g = 1.0f,
            .b = 0.2f,
            .a = 1.0f
    };
    const int32_t max_text_len = 49;
    char *buffer = oct_Malloc(g_game.allocator, max_text_len + 1);
    snprintf(buffer, max_text_len, "1%s%i%s%s%i%s%i", GLYPH_D8, pips, GLYPH_D6, GLYPH_ARROW, result, GLYPH_OUT_OF, dc);
    create_dice_label(buffer,
                      target_position,
                      passed ? green : red, true);

    return passed;
}
