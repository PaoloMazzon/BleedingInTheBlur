#include <math.h>
#include <stdio.h>
#include "Structs.h"
#include "Util.h"
#include "Game.h"
#include "AttackAnimations.h"
#include "Character.h"
#include "PersistentEffects.h"

const float PI = 3.141592635f;
const Oct_Colour WHITE = {
    .r = 1,
    .g = 1,
    .b = 1,
    .a = 1,
};

static void rotate_by_theta(Oct_Vec2 base, float theta) {
    // [cos(theta) -sin(theta)] [x]
    // [sin(theta)  cos(theta)] [y]
    const float cos_theta = cosf(theta);
    const float sin_theta = sinf(theta);
    const float x = base[0];
    const float y = base[1];
    base[0] = (cos_theta * x) + (-sin_theta * y);
    base[1] = (sin_theta * x) + ( cos_theta * y);
}

static void draw_ranged_animation() {
    // If it is a success, the projectile bounces off the target.
    // If it fails, the projectile flies past the target and fades out.
    Level *level = &g_game.current_level;
    const float normalized_time = 1.0f - timer_get_normalized(&level->Attack.animation_timer);
    const float drawn_rotation = g_game.current_level.Attack.projectile_rotation;
    const bool is_it_fadeout_time = normalized_time > g_game.current_level.Attack.percent_time_before_fadeout;
    Oct_Colour colour = {
            .r = 1.0f,
            .g = 1.0f,
            .b = 1.0f,
            .a = is_it_fadeout_time ? 1 - ((normalized_time - g_game.current_level.Attack.percent_time_before_fadeout) / (1 - g_game.current_level.Attack.percent_time_before_fadeout)) : 1.0f
    };

    // If we have hit the other character in the case of a successful attack
    if (is_it_fadeout_time && g_game.current_level.Attack.successful) {
        Oct_Vec2 current_velocity = {
                g_game.current_level.Attack.projectile_velocity[0],
                g_game.current_level.Attack.projectile_velocity[1]
        };
        rotate_by_theta(current_velocity, PI / 4);
        g_game.current_level.Attack.projectile_position[0] += current_velocity[0];
        g_game.current_level.Attack.projectile_position[1] += current_velocity[1];
        g_game.current_level.Attack.projectile_rotation += 0.2f;
    } else {
        g_game.current_level.Attack.projectile_position[0] += g_game.current_level.Attack.projectile_velocity[0];
        g_game.current_level.Attack.projectile_position[1] += g_game.current_level.Attack.projectile_velocity[1];
    }

    oct_DrawTextureIntColourExt(
            OCT_INTERPOLATE_ALL, PROJECTILE_ID,
            level->Attack.tex,
            &colour,
            g_game.current_level.Attack.projectile_position,
            (Oct_Vec2){1, 1},
            drawn_rotation,
            (Oct_Vec2){OCT_ORIGIN_MIDDLE, OCT_ORIGIN_MIDDLE});
}

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
        case ATTACK_ANIMATION_TYPE_RANGED:
            draw_ranged_animation();
            break;
        default:
            oct_Raise(OCT_STATUS_ERROR, true, "Unimplemented attack animation %i", g_game.current_level.Attack.attack_animation_type);
    }
}

// Handles more general traits like blur that have effects on
static void process_attack_traits() {
    Character *attacker = g_game.current_level.Attack.attacker;
    Character *defender = g_game.current_level.Attack.receiver;
    Traits *attack_traits = &g_game.current_level.Attack.traits;
    Traits *defender_traits = &defender->info.traits;

    if (attack_traits->Attack.blur) {
        character_take_damage(attacker, 2, &BLUR_SELF_DAMAGE_TRAITS, attacker);
    }
    if (attack_traits->Attack.rusted && random_int(0, 4) == 0) {
        character_take_damage(attacker, 1, &RUST_SELF_DAMAGE_TRAITS, attacker);
    } // TODO: Withering effect
    if (attack_traits->Attack.heavy && random_int(0, 4) == 0) {
        apply_crushed_hand_status_effect(defender);
        create_label("Crushed hand!", defender->pos, WHITE, false);
    }
}

static void complete_melee_or_ranged_attack() {
    // Some traits need to be applied regardless of attack success, but the rest doesn't need to happen
    process_attack_traits();
    if (!g_game.current_level.Attack.successful) return;

    const bool crit = random_int(1, 101) <= character_crit_chance(g_game.current_level.Attack.attacker);
    const int32_t multiplier = crit ? 2 : 1;
    const int32_t actual_damage = character_take_damage(
            g_game.current_level.Attack.receiver,
            g_game.current_level.Attack.damage * multiplier,
            &g_game.current_level.Attack.traits,
            g_game.current_level.Attack.attacker);
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
        case ATTACK_ANIMATION_TYPE_RANGED:
        case ATTACK_ANIMATION_TYPE_MELEE:
            complete_melee_or_ranged_attack();
            break;
        default:
            oct_Raise(OCT_STATUS_ERROR, true, "Unimplemented attack animation %i", g_game.current_level.Attack.attack_animation_type);
    }
}

void setup_melee_animation(Character *attacker, Character *receiver, const Traits *attack_traits, bool passed, int32_t damage) {
    g_game.current_level.Attack.attack_animation_type = ATTACK_ANIMATION_TYPE_MELEE;
    g_game.current_level.Attack.damage = damage;
    g_game.current_level.Attack.successful = passed;
    timer_start(&g_game.current_level.Attack.animation_timer, ATTACK_ANIMATION_DURATION);
    g_game.current_level.Attack.tex = OCT_NO_ASSET;
    g_game.current_level.Attack.attacker = attacker;
    g_game.current_level.Attack.receiver = receiver;
    g_game.current_level.Attack.traits = *attack_traits;
}

void setup_ranged_animation(Character *attacker, Character *receiver, const Traits *attack_traits, bool passed, int32_t damage, Oct_Texture projectile) {
    g_game.current_level.Attack.attack_animation_type = ATTACK_ANIMATION_TYPE_RANGED;
    g_game.current_level.Attack.damage = damage;
    g_game.current_level.Attack.successful = passed;
    timer_start(&g_game.current_level.Attack.animation_timer, ATTACK_ANIMATION_DURATION);
    g_game.current_level.Attack.tex = projectile;
    g_game.current_level.Attack.attacker = attacker;
    g_game.current_level.Attack.receiver = receiver;
    g_game.current_level.Attack.traits = *attack_traits;
    g_game.current_level.Attack.percent_time_before_fadeout = 0.6f;
    const float attacker_x = (float)attacker->pos[0] * CELL_WIDTH;
    const float attacker_y = (float)attacker->pos[1] * CELL_HEIGHT;
    const float receiver_x = (float)receiver->pos[0] * CELL_WIDTH;
    const float receiver_y = (float)receiver->pos[1] * CELL_HEIGHT;
    const float angle = oct_PointAngle(
            (Oct_Vec2){(float)attacker->pos[0] * CELL_WIDTH, (float)attacker->pos[1] * CELL_HEIGHT},
            (Oct_Vec2){(float)receiver->pos[0] * CELL_WIDTH, (float)receiver->pos[1] * CELL_HEIGHT});
    const float frames_before_fadeout = ((float)ATTACK_ANIMATION_DURATION * g_game.current_level.Attack.percent_time_before_fadeout);
    g_game.current_level.Attack.projectile_velocity[0] = (receiver_x - attacker_x) / frames_before_fadeout;
    g_game.current_level.Attack.projectile_velocity[1] = (receiver_y - attacker_y) / frames_before_fadeout;
    if (!passed) {
        rotate_by_theta(g_game.current_level.Attack.projectile_velocity, PI / 4);
    }
    g_game.current_level.Attack.projectile_position[0] = ((float)attacker->pos[0] * CELL_WIDTH) + 4;
    g_game.current_level.Attack.projectile_position[1] = ((float)attacker->pos[1] * CELL_HEIGHT) + 4;
    g_game.current_level.Attack.projectile_rotation = angle;
}
