// returns true if you should call it again
bool characters_update() {
    // FIXME: This should be made into a proper state machine. There are only 4 states: waiting to take turn (timer), taking turn (timer), in attack animation, take turn instantly
    if (get_level()->world_turn && get_level()->state == LEVEL_STATE_ENEMY_TURN) {
        Character *enemy = &get_level()->characters[get_level()->enemy_turn];

        // Delayed turn is done
        if (timer_tick(&get_level()->enemy_move_timer) && !level_in_attack_animation()) {
            level_next_enemy_turn();
            return true;
        }

        const bool done_turn_delay = timer_tick(&get_level()->enemy_delayed_turn_timer);
        if (done_turn_delay) {
            character_take_turn(enemy);
            timer_start(&get_level()->enemy_move_timer, 30 / 2);
        } else if (!timer_in_use(&get_level()->enemy_delayed_turn_timer) && !level_in_attack_animation() && !timer_in_use(&get_level()->enemy_move_timer)) {
            // Enemy turns will happen instantly if they either
            //  a) are > 5 tiles away from the player
            //  b) they have a wall between the player and them (slightly janky)
            //  c) the player has requested instant turns in the options TODO: This doesn't work
            if (tile_distance(enemy->pos, get_player()->pos) > 5 || tiles_have_walls_between(enemy->pos, get_player()->pos) || !get_game()->options.animate_enemy_movement) {
                // Take turn instantly
                character_take_turn(enemy);
                level_next_enemy_turn();
                return true;
            } else {
                timer_start(&get_level()->enemy_delayed_turn_timer, 30 / 2);
            }
        } else if (!level_in_attack_animation() && (timer_in_use(&get_level()->enemy_move_timer) || timer_in_use(&get_level()->enemy_delayed_turn_timer))) {
            // Center camera on the enemy thats moving around
            look_at(enemy->pos, 1);
        } else if (level_attack_animation_complete()) {
            level_next_enemy_turn();
        } // TODO: Only one enemy seems to be able to attack per turn sometimes
    } else if (!get_level()->world_turn && get_level()->state == LEVEL_STATE_ENEMY_TURN && !level_in_attack_animation()) {
        get_level()->state = LEVEL_STATE_PLAYER_INTERACTION;
    }
    return false;
}

// This will also move characters to where they are supposed to be in the game world
void draw_characters() {
    get_level()->known_character_location_count = 0;
    for (int i = 0; i < MAX_CHARACTERS + 1; i++) {
        // Accounts for player as the last character to draw
        Character *c = nullptr;
        if (i == MAX_CHARACTERS) c = get_player();
        else c = &get_level()->characters[i];
        ObjectInfo *c_info = &c->info;

        if (!character_is_alive(c)) continue;
        const float target_alpha = level_tile_seen_this_turn(c->pos) ? 1 : 0;
        c_info->actual_alpha += (target_alpha - c_info->actual_alpha) * 0.4f;
        // i dont know why clang thinks target_alpha is always false but testing proves this is incorrect
        if (c != get_player() && target_alpha == 0 && character_is_aware_of_other_character(get_player(), c, false)) {
            get_level()->known_character_locations[get_level()->known_character_location_count] = c;
            get_level()->known_character_location_count += 1;
        }

        // Move character to where they should be
        const Oct_Vec2 velocity = {
                (c_info->target_position[0] - c_info->actual_position[0]) * 0.4f,
                (c_info->target_position[1] - c_info->actual_position[1]) * 0.4f,
        };
        c_info->actual_position[0] += velocity[0];
        c_info->actual_position[1] += velocity[1];

        // Make sure the facing value accurately reflects where the character is pointing
        if (!timer_in_use(&c->face_away_timer)) {
            if (velocity[0] > 0.01)
                c_info->facing_direction = 1;
            else if (velocity[0] < -0.01)
                c_info->facing_direction = -1;

            // Lerp scale to match facing
            c_info->scale_x += (c_info->facing_direction - c_info->scale_x) * 0.35f;
        }

        // Cool rotation effect
        const float speed = sqrtf(powf(velocity[0], 2) + powf(velocity[1], 2));
        c_info->rotation = speed * c_info->facing_direction * 0.15f;

        character_draw(c, c_info->actual_position, c_info->actual_alpha);
        timer_tick(&c->face_away_timer);
    }

    // Draw the attack animation
    if (level_in_attack_animation()) {
        draw_attack_animation();
    }
}

// If the player is aware of a character for some reason but can't see them
void draw_character_outlines() {
    for (int32_t i = 0; i < get_level()->known_character_location_count; i++) {
        Character *c = get_level()->known_character_locations[i];
        oct_DrawTextureInt(
                OCT_INTERPOLATE_ALL, c->info.id * OBJECT_ID_RESERVED_MULTIPLIER,
                oct_GetAsset(get_assets(), "characters/sharpcharacter.png"),
                (Oct_Vec2){c->info.actual_position[0], c->info.actual_position[1]});
    }
}
