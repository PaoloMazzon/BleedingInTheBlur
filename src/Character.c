#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "Character.h"
#include "Util.h"
#include "Game.h"
#include "AttackAnimations.h"
#include "PersistentEffects.h"

void base_statblock(Statblock *sb) {
    memset(sb, 0, sizeof(Statblock));

    for (int32_t base_stat = 0; base_stat < 4; base_stat++) {
        sb->base_stats[base_stat] = 2;
        for (int32_t sub_stat = 0; sub_stat < 4; sub_stat++) {
            int32_t *skill = get_skill_pip(sb, base_stat, sub_stat);
            *skill = 1;
        }
    }
}

void random_statblock(Statblock *sb) {
    base_statblock(sb);

    // Pick base stats first
    for (int i = 0; i < STARTING_BASE_STAT_POINTS;) {
        const int32_t stat = random_int(0, 4);
        if (sb->base_stats[stat] >= BASE_STAT_MAX) continue;
        sb->base_stats[stat] += 1;
        i++;
    }

    // Pick skill pips
    for (int i = 0; i < STARTING_SKILL_PIPS; i++) {
        const int32_t base_stat = random_int(0, 4);
        const int32_t skill = random_int(0, 4);
        int32_t *skill_stat = get_skill_pip(sb, base_stat, skill);
        *skill_stat += 1;
    }
}

void print_statblock(Statblock *s) {
    char skill_score[26] = "=========================";
    char skill_name_offset[13] = "            ";
    for (int base_stat = 0; base_stat < 4; base_stat++) {
        debug("---------- %s: %i", BASE_STAT_NAMES[base_stat], s->base_stats[base_stat]);
        for (int skill = 0; skill < 4; skill++) {
            const int32_t skill_pips = *get_skill_pip(s, base_stat, skill);
            const char *skill_name = get_skill_name(base_stat, skill);
            skill_score[skill_pips] = '\0';
            skill_name_offset[12 - strlen(skill_name)] = '\0'; // might cause problems on a rename
            // If this fails that means a name was changed without updating the skill_name_offset size
            assert(strlen(skill_name) <= 12);
            debug("%s%s |%s", skill_name_offset, skill_name, skill_score);
            skill_score[skill_pips] = '=';
            skill_name_offset[12 - strlen(skill_name)] = ' ';
        }
    }
    debug("----------");
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

void character_draw(Character *c, Oct_Vec2 position, float alpha) {
    draw_object(&c->info, position, 1, alpha);
}

void draw_object(ObjectInfo *info, Oct_Vec2 position, float scale, float alpha) {
    if (info->drawn_type == DRAWN_TYPE_SPRITE) {
        oct_DrawSpriteIntColourExt(
                OCT_INTERPOLATE_POSITION | OCT_INTERPOLATE_ROTATION, info->id,
                info->sprite, &info->sprite_instance,
                &(Oct_Colour){.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = alpha},
                (Oct_Vec2){position[0] + 4, position[1] + 4},
                (Oct_Vec2){info->scale_x * scale, scale},
                info->rotation, (Oct_Vec2){OCT_ORIGIN_MIDDLE, OCT_ORIGIN_MIDDLE});
    } else if (info->drawn_type == DRAWN_TYPE_TEXTURE) {
        oct_DrawTextureIntColourExt(
                OCT_INTERPOLATE_POSITION | OCT_INTERPOLATE_ROTATION, info->id,
                info->texture,
                &(Oct_Colour){.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = alpha},
                (Oct_Vec2){position[0] + 4, position[1] + 4},
                (Oct_Vec2){info->scale_x * scale, scale},
                info->rotation, (Oct_Vec2){OCT_ORIGIN_MIDDLE, OCT_ORIGIN_MIDDLE});
    }
}

static void draw_item_charges(Item *item, const Oct_Vec2 position, bool interpolate) {
    float x = position[0] - 1;
    float y = position[1] - 1;
    if (item->charges == 1) return; // dont draw only 1 charge, thats a given
    for (int32_t i = 0; i < item->charges_remaining; i++) {
        oct_DrawTextureInt(
                interpolate ? OCT_INTERPOLATE_ALL : 0, (item->info.id * OBJECT_ID_RESERVED_MULTIPLIER) + i,
                oct_GetAsset(g_game.assets, "hud/itemchargepip.png"),
                (Oct_Vec2){x, y});
        y += 2;
    }
}

void draw_item(Item *item, Oct_Vec2 position, float alpha) {
    draw_object_raw(&item->info, position, 2, alpha);
    draw_item_charges(item, position, true);
}

void draw_item_no_int(Item *item, Oct_Vec2 position, float alpha) {
    draw_object_raw_no_int(&item->info, position, 2, alpha);
    draw_item_charges(item, position, false);
}

void draw_object_raw(ObjectInfo *info, Oct_Vec2 position, float scale, float alpha) {
    if (info->drawn_type == DRAWN_TYPE_SPRITE) {
        oct_DrawSpriteIntColourExt(
                OCT_INTERPOLATE_ALL, info->id,
                info->sprite, &info->sprite_instance,
                &(Oct_Colour){.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = alpha},
                (Oct_Vec2){position[0], position[1]},
                (Oct_Vec2){scale, scale},
                0, (Oct_Vec2){0, 0});
    } else if (info->drawn_type == DRAWN_TYPE_TEXTURE) {
        oct_DrawTextureIntColourExt(
                OCT_INTERPOLATE_ALL, info->id,
                info->texture,
                &(Oct_Colour){.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = alpha},
                (Oct_Vec2){position[0], position[1]},
                (Oct_Vec2){scale, scale},
                0, (Oct_Vec2){0, 0});
    }
}

void draw_object_raw_no_int(ObjectInfo *info, Oct_Vec2 position, float scale, float alpha) {
    if (info->drawn_type == DRAWN_TYPE_SPRITE) {
        oct_DrawSpriteColourExt(
                info->sprite, &info->sprite_instance,
                &(Oct_Colour){.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = alpha},
                (Oct_Vec2){position[0], position[1]},
                (Oct_Vec2){scale, scale},
                0, (Oct_Vec2){0, 0});
    } else if (info->drawn_type == DRAWN_TYPE_TEXTURE) {
        oct_DrawTextureColourExt(
                info->texture,
                &(Oct_Colour){.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = alpha},
                (Oct_Vec2){position[0], position[1]},
                (Oct_Vec2){scale, scale},
                0, (Oct_Vec2){0, 0});
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
    return c->bonus_max_hp + (current_statblock.grit * 3) + 10;
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

int32_t character_take_damage(Character *c, int32_t damage, Traits *source_traits, Character *attacker) {
    Statblock current_statblock;
    character_get_current_stats(c, &current_statblock);
    const int32_t initial = c->current_hp;

    // They aggro no matter what
    if (attacker && attacker != c) {
        c->aggro_timer = 3;
        c->aggrod_character = attacker;
    }

    // Cooldown for death's door
    if (c->deaths_door_cooldown > 0) c->deaths_door_cooldown -= 1;

    // They can evade
    if (!source_traits->Attack.intricate && roll_ups(current_statblock.evade, 2, nullptr) && attacker != c) {
        create_label("Evaded!", c->pos, (Oct_Colour){.r = 0.3f, .g = 1.0f, .b = 0.3f, .a = 1.0f}, false);
        return 0;
    } else if (source_traits->Attack.intricate && roll_ups(current_statblock.escape, 2, nullptr) && attacker != c) {
        create_label("Evaded!", c->pos, (Oct_Colour){.r = 0.3f, .g = 1.0f, .b = 0.3f, .a = 1.0f}, false);
        return 0;
    }
    // They can roll death's door if they would die
    if (c->current_hp - damage <= 0
    && !c->info.traits.Character.undying
    && roll_ups(current_statblock.deaths_door, 1, nullptr)
    && c->deaths_door_cooldown == 0) {
        c->current_hp = 1;
        c->deaths_door_cooldown = 3;
        create_label("Momento mori", c->pos, (Oct_Colour){.r = 0.3f, .g = 1.0f, .b = 0.3f, .a = 1.0f}, false);
        return 0;
    }

    c->current_hp = non_negative(c->current_hp - damage);

    // check if they need to get deleted for dying and also trigger the blood_thirsty trait
    if (!character_is_alive(c)) {
        TileContents *t = level_get_tile(c->pos);
        if (t && t->type == TILE_CONTENTS_TYPE_CHARACTER && t->character == c) {
            t->type = TILE_CONTENTS_TYPE_NONE;
            t->character = nullptr;
        }
        // Handle healing for attackers that have the blood_thirsty trait
        if (attacker && attacker->info.traits.Character.blood_thirsty)
            character_heal(attacker, 2);
    }

    return initial - c->current_hp;
}

// rolls healing (against grit) and on a pass there is bonus hp
static int32_t get_bonus_hp(Character *c) {
    Statblock sb;
    character_get_current_stats(c, &sb);
    int32_t result = 0;
    int32_t dice_result = 0;
    if (roll_dice(sb.healing, statblock_get_dc(sb.grit), &dice_result))
        result = dice_result - statblock_get_dc(sb.grit) + 1;
    return result;
}

int32_t character_heal(Character *c, int32_t healing) {
    const int32_t bonus_healing = get_bonus_hp(c);
    const int32_t max_hp = character_max_hp(c);
    const int32_t starting_hp = c->current_hp;
    c->current_hp += healing + bonus_healing;
    if (c->current_hp > max_hp)
        c->current_hp = max_hp;
    return c->current_hp - starting_hp;
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
    if (next_tile && ((next_tile->type == TILE_CONTENTS_TYPE_WALL && !next_tile->tile.door) || next_tile->type == TILE_CONTENTS_TYPE_CHARACTER)) {
        return false;
    } else if (next_tile && (next_tile->type == TILE_CONTENTS_TYPE_WALL && next_tile->tile.door && !next_tile->tile.door_open)) {
        next_tile->type = TILE_CONTENTS_TYPE_NONE;
        oct_SetTilemap(g_game.current_level.decorations, new_position[0], new_position[1], TILE_DOOR_OPEN);
        return true;
    }

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

void character_get_attack_base_stats(Character *c, const Traits *attack_traits, int32_t *out_pips, int32_t *out_dc) {
    Statblock current;
    character_get_current_stats(c, &current);
    int32_t pip_count = 0;
    int32_t dc = attack_traits->Attack.intricate ? statblock_get_dc(current.learning) : statblock_get_dc(current.martial);
    if (attack_traits->Attack.melee && !attack_traits->Attack.improvised)
        pip_count = current.blades;
    else if (attack_traits->Attack.ranged)
        pip_count = current.marksman;
    else
        pip_count = current.grappler;
    if (out_dc) *out_dc = dc;
    if (out_pips) *out_pips = pip_count;
}

AttackFavour character_get_attack_stats(Character *c, const Traits *attack_traits, Position target_position, const Traits *target_traits, int32_t *out_pips, int32_t *out_dc) {
    int32_t pip_count;
    int32_t dc;
    character_get_attack_base_stats(c, attack_traits, &pip_count, &dc);
    const int32_t distance = tile_distance(c->pos, target_position);
    const int32_t base_pips = pip_count;
    const TileContents *target_tile = level_get_tile(target_position);
    const Character *defender = target_tile->type == TILE_CONTENTS_TYPE_CHARACTER ? target_tile->character : nullptr;
    const int32_t defender_hp = defender ? defender->current_hp : 0;
    const int32_t defender_max_hp = defender ? character_max_hp(defender) : 0;
    const bool target_is_previous_attacker = defender && defender == c->aggrod_character && c->aggro_timer > 0;

    // Bonuses and detriments
    if (target_traits->Character.nimble && attack_traits->Attack.ranged) pip_count -= 1;
    if (target_traits->Character.lazy && attack_traits->Attack.heavy) pip_count += 1;
    if (target_traits->occult && attack_traits->holy) pip_count += 1;
    if (target_traits->holy && attack_traits->occult) pip_count += 1;
    if (tile_distance(target_position, c->pos) == 1 && attack_traits->Attack.ranged) pip_count -= 1;
    if (attack_traits->Attack.blur) pip_count += 2;
    if (target_traits->Character.dumb && attack_traits->Attack.blood) pip_count += 1;
    if (target_traits->Character.wet && attack_traits->Attack.lightning) pip_count += 1;
    if (target_traits->Character.abyssal && defender_hp < defender_max_hp * 0.3) pip_count -= 1;
    if (c->info.traits.Character.dumb && random_int(0, 4) == 0) pip_count -= 1;
    if (target_traits->Attack.vengeful && target_is_previous_attacker) pip_count += 1;
    if (attack_traits->Attack.precise && attack_traits->Attack.melee && distance == 1) pip_count += 1;
    if (attack_traits->Attack.precise && attack_traits->Attack.ranged && distance == 1) pip_count -= 1;
    if (attack_traits->Attack.precise && attack_traits->Attack.ranged && distance >= 3) pip_count += 1;

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

    if (attack_traits->Attack.ranged)
        setup_ranged_animation(c, rcvr, attack_traits, passed, bonus_damage + base_attack_damage, oct_GetAsset(g_game.assets, "items/rock.png"));
    else if (attack_traits->Attack.melee)
        setup_melee_animation(c, rcvr, attack_traits, passed, bonus_damage + base_attack_damage);
    else
        oct_Raise(OCT_STATUS_ERROR, true, "Attempted to start an attack animation for a non-ranged, non-melee attack");

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
                      (Position){target_position[0], target_position[1] + 2},
                      passed ? green : red, true);

    return passed;
}

void character_process_alarms(Character *c) {
    for (int32_t i = 0; i < MAX_ALARMS; i++) {
        if (alarm_is_active(c, i)) {
            c->alarms[i].turns_left -= 1;
            if (c->alarms[i].turns_left == 0) {
                c->alarms[i].callback(c);
            }
        }
    }
}

bool character_is_aware_of_other_character(Character *c, Character *other, bool consider_aggro_range) {
    const int32_t distance = tile_distance(c->pos, other->pos);
    const bool heightened_senses = c->info.traits.Character.sharp;
    const bool within_aggro_range = !consider_aggro_range || (distance <= c->aggro_range);
    const bool line_of_sight = !tiles_have_walls_between(c->pos, other->pos) || !tiles_have_walls_between(other->pos, c->pos);
    return (distance <= 3 && heightened_senses) || (within_aggro_range && line_of_sight);
}
