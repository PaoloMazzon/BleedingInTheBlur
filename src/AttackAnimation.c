#include <math.h>
#include <stdio.h>
#include "Structs.h"
#include "Util.h"
#include "Game.h"
#include "AttackAnimations.h"
#include "Character.h"

static void draw_ranged_animation() {
    // If it is a success, the projectile bounces off the target.
    // If it fails, the projectile flies past the target and fades out.
    Level *level = &g_game.current_level;
    const float start_x = (float)level->Attack.attacker->pos[0] * CELL_WIDTH;
    const float start_y = (float)level->Attack.attacker->pos[1] * CELL_HEIGHT;
    const float normalized_time = timer_get_normalized(&level->Attack.animation_timer);
    const float frames_passed = normalized_time * 30;
    float drawn_rotation = level->Attack.rotation;
    const float computed_cast_x = cosf(level->Attack.rotation + (3.141592635f / 6));
    const float computed_cast_y = sinf(level->Attack.rotation + (3.141592635f / 6));
    Oct_Colour colour = {
            .r = 1.0f,
            .g = 1.0f,
            .b = 1.0f,
            .a = 1.0f,
    };
    Oct_Vec2 drawn_location = {
            start_x + (computed_cast_x * level->Attack.speed * frames_passed),
            start_y + (computed_cast_y * level->Attack.speed * frames_passed),
    };
    if (level->Attack.successful && normalized_time > level->Attack.percent_time_before_fadeout) {
        drawn_location[0] = start_x + (computed_cast_x * level->Attack.speed * 30 * level->Attack.percent_time_before_fadeout);
        drawn_location[1] = start_y + (computed_cast_y * level->Attack.speed * 30 * level->Attack.percent_time_before_fadeout);
        const float excess_frames = (float)ATTACK_ANIMATION_DURATION - frames_passed;
        drawn_location[0] += (computed_cast_x * level->Attack.speed * excess_frames * 0.5f);
        drawn_location[1] += (computed_cast_y * level->Attack.speed * excess_frames * 0.5f);
        drawn_rotation += 0.3f;
    }
    if (normalized_time > level->Attack.percent_time_before_fadeout)
        colour.a = (normalized_time - level->Attack.percent_time_before_fadeout) / (1 - level->Attack.percent_time_before_fadeout);
    oct_DrawTextureIntColourExt(
            OCT_INTERPOLATE_ALL, PROJECTILE_ID,
            level->Attack.tex,
            &colour,
            drawn_location,
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

static void complete_melee_or_ranged_attack() {
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
    g_game.current_level.Attack.percent_time_before_fadeout = 0.8f;
    const float angle = oct_PointAngle((Oct_Vec2){(float)attacker->pos[0] * CELL_WIDTH, (float)attacker->pos[1] * CELL_HEIGHT}, (Oct_Vec2){(float)receiver->pos[0] * CELL_WIDTH, (float)receiver->pos[1] * CELL_HEIGHT});
    g_game.current_level.Attack.rotation = passed ? angle : angle - 0.2f;
    g_game.current_level.Attack.speed = oct_PointDistance((Oct_Vec2){(float)attacker->pos[0] * CELL_WIDTH, (float)attacker->pos[1] * CELL_HEIGHT}, (Oct_Vec2){(float)receiver->pos[0] * CELL_WIDTH, (float)receiver->pos[1] * CELL_HEIGHT}) / ((float)ATTACK_ANIMATION_DURATION * g_game.current_level.Attack.percent_time_before_fadeout);
}