#include <stdio.h>
#include "Structs.h"
#include "Util.h"
#include "Game.h"
#include "AttackAnimations.h"
#include "Character.h"

static void draw_melee_animation() {
    Level *level = &g_game.current_level;
    Position rcvr_target_tile = {
            level->Attack.receiver->pos[0] + 1,
            level->Attack.receiver->pos[1],
    };
    if (level->Attack.receiver->pos[0] < level->Attack.attacker->pos[0])
        rcvr_target_tile[0] = level->Attack.receiver->pos[0] - 1;
    if (level->Attack.successful) rcvr_target_tile[0] = level->Attack.receiver->pos[0];

    Position attacker_target_tile = {
            level->Attack.receiver->pos[0],
            level->Attack.receiver->pos[1],
    };
    const float interpolation = hyperbolic_x(timer_get_normalized(&level->Attack.animation_timer));

    // Interpolate target position and effects for attacker
    ObjectInfo *atk_info = &level->Attack.attacker->info;
    ObjectInfo *rcvr_info = &level->Attack.receiver->info;
    atk_info->actual_position[0] = atk_info->actual_position[0] + (((float)attacker_target_tile[0] * CELL_WIDTH) - atk_info->actual_position[0]) * interpolation;
    atk_info->actual_position[1] = atk_info->actual_position[1] + (((float)attacker_target_tile[1] * CELL_HEIGHT) - atk_info->actual_position[1]) * interpolation;
    atk_info->rotation = interpolation * atk_info->facing_direction * 0.5f;

    // Account for facing the wrong way
    atk_info->facing_direction = (level->Attack.attacker->pos[0] <= level->Attack.receiver->pos[0]) ? 1 : -1;
    atk_info->scale_x = atk_info->scale_x;
    timer_start(&level->Attack.attacker->face_away_timer, 20);

    // Interpolate target position and effects for receiver
    rcvr_info->actual_position[0] = rcvr_info->actual_position[0] + (((float)rcvr_target_tile[0] * CELL_WIDTH) - rcvr_info->actual_position[0]) * interpolation;
    rcvr_info->actual_position[1] = rcvr_info->actual_position[1] + (((float)rcvr_target_tile[1] * CELL_HEIGHT) - rcvr_info->actual_position[1]) * interpolation;
    rcvr_info->rotation = interpolation * -rcvr_info->facing_direction * 0.5f;
}

void draw_attack_animation() {
    switch (g_game.current_level.Attack.attack_animation_type) {
        case ATTACK_ANIMATION_TYPE_MELEE:
            draw_melee_animation();
            break;
        default:
            oct_Raise(OCT_STATUS_ERROR, true, "Unimplemented attack animation %i", g_game.current_level.Attack.attack_animation_type);
    }
}

static void complete_melee_attack() {
    if (!g_game.current_level.Attack.successful) return;
    const bool crit = random_int(1, 101) <= character_crit_chance(g_game.current_level.Attack.attacker);
    const int32_t multiplier = crit ? 2 : 1;
    const int32_t actual_damage = character_take_damage(
            g_game.current_level.Attack.receiver,
            g_game.current_level.Attack.damage * multiplier,
            &g_game.current_level.Attack.traits);
    if (actual_damage != 0) {
        if (!crit)
            snprintf(g_game.current_level.Attack.buffer, MAX_BUFFER_LENGTH - 1, "%i dmg", actual_damage);
        else
            snprintf(g_game.current_level.Attack.buffer, MAX_BUFFER_LENGTH - 1, "CRIT %i!", actual_damage);
        Oct_Colour c = {
                .r = 1.0,
                .g = 1.0,
                .b = 1.0,
                .a = 1.0,
        };
        create_label(
                g_game.current_level.Attack.buffer,
                g_game.current_level.Attack.receiver->pos,
                c,
                false);
    }
}

void complete_attack_animation() {
    switch (g_game.current_level.Attack.attack_animation_type) {
        case ATTACK_ANIMATION_TYPE_MELEE:
            complete_melee_attack();
            break;
        default:
            oct_Raise(OCT_STATUS_ERROR, true, "Unimplemented attack animation %i", g_game.current_level.Attack.attack_animation_type);
    }
}

void setup_melee_animation(Character *attacker, Character *receiver, const Traits *attack_traits, bool passed, int32_t damage) {
    // Setup the level attack animation
    g_game.current_level.Attack.attack_animation_type = ATTACK_ANIMATION_TYPE_MELEE;
    g_game.current_level.Attack.damage = damage;
    g_game.current_level.Attack.successful = passed;
    timer_start(&g_game.current_level.Attack.animation_timer, ATTACK_ANIMATION_DURATION);
    g_game.current_level.Attack.tex = OCT_NO_ASSET;
    g_game.current_level.Attack.attacker = attacker;
    g_game.current_level.Attack.receiver = receiver;
    g_game.current_level.Attack.traits = *attack_traits;
}