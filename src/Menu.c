#include <string.h>
#include <math.h>
#include <oct/Octarine.h>

#include "Character.h"
#include "Game.h"
#include "MenuSystem.h"
#include "MenuDetails.h"
#include "Constants.h"

static MenuSystem test_menu = {0};
static const int32_t TAB_COUNT = 4;
static MenuSystemTab *tab_start;
static const int32_t tab_index_start = 0;
static MenuSystemTab *tab_options;
static const int32_t tab_index_options = 1;
static MenuSystemTab *tab_character;
static const int32_t tab_index_character = 2;
static MenuSystemTab *tab_stats;
static const int32_t tab_index_stats = 3;
static bool should_play_game = false;

static Statblock player_starting_statblock;
static int32_t player_available_base_points;
static int32_t player_available_skill_points;

/********************************* Top-level menu/shared *********************************/

static void play_change_callback(int32_t _) {
    debug("Pressed play");
    //should_play_game = true; // TODO: Debug bullshit
    menu_set_tab(&test_menu, tab_index_character);
}

static void settings_change_callback(int32_t index) {
    debug("Went to settings screen");
    menu_set_tab(&test_menu, tab_index_options);
}

static void exit_change_callback(int32_t _) {
    oct_Log("Quit game from main menu.");
    exit(0);
}

static void back_change_callback(int32_t _) {
    debug("Returned to main screen");
    menu_set_tab(&test_menu, tab_index_start);
}

static void back_to_character_change_callback(int32_t _) {
    debug("Returned to main screen");
    menu_set_tab(&test_menu, tab_index_character);
}

/********************************* Character creation *********************************/

static void body_change_callback(int32_t index) {
    g_game.player.info.sprite.layers[SPRITE_LAYER_BODY] = index;
}
static void shoes_change_callback(int32_t index) {
    g_game.player.info.sprite.layers[SPRITE_LAYER_SHOES] = index;
}
static void pants_change_callback(int32_t index) {
    g_game.player.info.sprite.layers[SPRITE_LAYER_PANTS] = index;
}
static void shirt_change_callback(int32_t index) {
    g_game.player.info.sprite.layers[SPRITE_LAYER_SHIRT] = index;
}
static void head_change_callback(int32_t index) {
    g_game.player.info.sprite.layers[SPRITE_LAYER_HEAD] = index;
}
static void accessory_change_callback(int32_t index) {
    g_game.player.info.sprite.layers[SPRITE_LAYER_ACCESSORY] = index;
}
static void body_colour_change_callback(int32_t index) {
    g_game.player.info.sprite.layer_colours[SPRITE_LAYER_BODY] = index;
}
static void shoes_colour_change_callback(int32_t index) {
    g_game.player.info.sprite.layer_colours[SPRITE_LAYER_SHOES] = index;
}
static void pants_colour_change_callback(int32_t index) {
    g_game.player.info.sprite.layer_colours[SPRITE_LAYER_PANTS] = index;
}
static void shirt_colour_change_callback(int32_t index) {
    g_game.player.info.sprite.layer_colours[SPRITE_LAYER_SHIRT] = index;
}
static void head_colour_change_callback(int32_t index) {
    g_game.player.info.sprite.layer_colours[SPRITE_LAYER_HEAD] = index;
}
static void accessory_colour_change_callback(int32_t index) {
    g_game.player.info.sprite.layer_colours[SPRITE_LAYER_ACCESSORY] = index;
}
static void next_change_callback(int32_t _) {
    menu_set_tab(&test_menu, tab_index_stats);
}
static void random_character_callback(int32_t _) {
    info_set_random_sprite_layers(&g_game.player.info);
}

/********************************* Stat allocation *********************************/

// Evil piece of shit code generation
#define STAT_CALLBACK(stat, min, max) static void stats_##stat##_callback(int32_t direction) { \
    if (player_starting_statblock.stat < max && player_available_skill_points > 0 && direction == 1) { \
        player_starting_statblock.stat += 1;                                     \
        player_available_skill_points -= 1;                                      \
    } else if (direction == -1 && player_starting_statblock.stat > min) {        \
        player_starting_statblock.stat -= 1;                                     \
        player_available_skill_points += 1;                                      \
    }                                                                            \
}

#define BASE_STAT_CALLBACK(stat, min, max) static void stats_##stat##_callback(int32_t direction) { \
    if (player_starting_statblock.stat < max && player_available_base_points > 0 && direction == 1) { \
        player_starting_statblock.stat += 1;                                     \
        player_available_base_points -= 1;                                       \
    } else if (direction == -1 && player_starting_statblock.stat > min) {        \
        player_starting_statblock.stat -= 1;                                     \
        player_available_base_points += 1;                                       \
    }                                                                            \
}

BASE_STAT_CALLBACK(wits, 2, BASE_STAT_MAX)
STAT_CALLBACK(perception, 2, 5)
STAT_CALLBACK(escape, 2, 5)
STAT_CALLBACK(deception, 2, 5)
STAT_CALLBACK(trapping, 2, 5)
BASE_STAT_CALLBACK(grit, 2, BASE_STAT_MAX)
STAT_CALLBACK(suffer, 2, 5)
STAT_CALLBACK(healing, 2, 5)
STAT_CALLBACK(deaths_door, 2, 5)
STAT_CALLBACK(attrition, 2, 5)
BASE_STAT_CALLBACK(martial, 2, BASE_STAT_MAX)
STAT_CALLBACK(blades, 2, 5)
STAT_CALLBACK(marksman, 2, 5)
STAT_CALLBACK(grappler, 2, 5)
STAT_CALLBACK(evade, 2, 5)
BASE_STAT_CALLBACK(learning, 2, BASE_STAT_MAX)
STAT_CALLBACK(occult, 2, 5)
STAT_CALLBACK(herbalism, 2, 5)
STAT_CALLBACK(tactics, 2, 5)
STAT_CALLBACK(cartography, 2, 5)

/********************************* Actual menu logic *********************************/
void menu_begin() {
    memset(&player_starting_statblock, 0, sizeof(Statblock));
    player_available_base_points = STARTING_BASE_STAT_POINTS;
    player_available_skill_points = STARTING_SKILL_PIPS;
    for (int32_t base_stat = 0; base_stat < 4; base_stat++) {
        for (int32_t skill = 0; skill < 4; skill++) {
            *get_skill_pip(&player_starting_statblock, base_stat, skill) = MINIMUM_PIPS_PER_SKILL;
        }
        player_starting_statblock.base_stats[base_stat] = MINIMUM_PIPS_PER_BASE_SKILL;
    }

    // Setup the player to be able to be drawn without properly initializing the full character
    info_set_random_sprite_layers(&g_game.player.info);
    g_game.player.info.scale_x = 1;

    menu_system_initialize(&test_menu, 4);
    tab_start = menu_get_tab(&test_menu, tab_index_start);
    tab_options = menu_get_tab(&test_menu, tab_index_options);
    tab_character = menu_get_tab(&test_menu, tab_index_character);
    tab_stats = menu_get_tab(&test_menu, tab_index_stats);
    MenuOption option_play = {
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {20, 175},
        .name = "Play",
        .draw_callback = nullptr,
        .change_callback = play_change_callback,
    };
    MenuOption option_cycle = {
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {20, 195},
        .name = "Options",
        .draw_callback = nullptr,
        .change_callback = settings_change_callback,
    };
    MenuOption option_exit = {
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {20, 215},
        .name = "Exit",
        .draw_callback = nullptr,
        .change_callback = exit_change_callback,
    };
    MenuOption option_back = {
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {20, 90 + 20 * 7},
        .name = "Back",
        .draw_callback = nullptr,
        .change_callback = back_change_callback,
    };
    menu_tab_add_option(tab_start, &option_play, 0, (Position){0, 0});
    menu_tab_add_option(tab_start, &option_cycle, 0, (Position){0, 1});
    menu_tab_add_option(tab_start, &option_exit, 0, (Position){0, 2});

    menu_tab_add_option(tab_options, &music_volume_option, (int32_t)g_game.options.music_volume * 10, (Position){0, 0});
    menu_tab_add_option(tab_options, &sfx_volume_option, (int32_t)g_game.options.music_volume * 10, (Position){0, 1});
    menu_tab_add_option(tab_options, &animation_speed_option, g_game.options.animation_speed, (Position){0, 2});
    menu_tab_add_option(tab_options, &auto_pick_up_item_option, 0, (Position){0, 3});
    menu_tab_add_option(tab_options, &animate_enemy_movement_option, 0, (Position){0, 4});
    menu_tab_add_option(tab_options, &fullscreen_option, 0, (Position){0, 5});
    menu_tab_add_option(tab_options, &scale_mode_option, g_game.options.scale_mode, (Position){0, 6});
    menu_tab_add_option(tab_options, &option_back, 0, (Position){0, 7});


    const float body_x = 30;
    const float body_second_x = 220;
    const float body_start_y = 70;
    float body_y = body_start_y;
    const float body_increment_y = 20;
    const MenuOption option_body_change = {
        .name = "Body",
        .max_index = MAX_SPRITE_OPTIONS_PER_LAYER[SPRITE_LAYER_BODY],
        .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
        .drawn_position = {body_second_x, body_y},
        .change_callback = body_change_callback,
    };
    body_y += body_increment_y;
    const MenuOption option_shoes_change = {
        .name = "Shoes",
        .max_index = MAX_SPRITE_OPTIONS_PER_LAYER[SPRITE_LAYER_SHOES],
        .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
        .drawn_position = {body_x, body_y},
        .change_callback = shoes_change_callback,
    };
    body_y += body_increment_y;
    const MenuOption option_pants_change = {
        .name = "Pants",
        .max_index = MAX_SPRITE_OPTIONS_PER_LAYER[SPRITE_LAYER_PANTS],
        .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
        .drawn_position = {body_x, body_y},
        .change_callback = pants_change_callback,
    };
    body_y += body_increment_y;
    const MenuOption option_shirt_change = {
        .name = "Shirt",
        .max_index = MAX_SPRITE_OPTIONS_PER_LAYER[SPRITE_LAYER_SHIRT],
        .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
        .drawn_position = {body_x, body_y},
        .change_callback = shirt_change_callback,
    };
    body_y += body_increment_y;
    const MenuOption option_head_change = {
        .name = "Head",
        .max_index = MAX_SPRITE_OPTIONS_PER_LAYER[SPRITE_LAYER_HEAD],
        .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
        .drawn_position = {body_x, body_y},
        .change_callback = head_change_callback,
    };
    body_y += body_increment_y;
    const MenuOption option_accessory_change = {
        .name = "Accessory",
        .max_index = MAX_SPRITE_OPTIONS_PER_LAYER[SPRITE_LAYER_ACCESSORY],
        .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
        .drawn_position = {body_x, body_y},
        .change_callback = accessory_change_callback,
    };
    body_y = body_start_y;
    const MenuOption option_character_creation_back = {
        .name = "Back",
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {body_x, body_y},
        .change_callback = back_change_callback,
    };
    body_y += body_increment_y;
    const MenuOption option_shoes_colour_change = {
        .name = "S-Colour",
        .max_index = MAX_COLOURS,
        .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
        .drawn_position = {body_second_x, body_y},
        .change_callback = shoes_colour_change_callback,
    };
    body_y += body_increment_y;
    const MenuOption option_pants_colour_change = {
        .name = "P-Colour",
        .max_index = MAX_COLOURS,
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {body_second_x, body_y},
        .change_callback = pants_colour_change_callback,
    };
    body_y += body_increment_y;
    const MenuOption option_shirt_colour_change = {
        .name = "S-Colour",
        .max_index = MAX_COLOURS,
        .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
        .drawn_position = {body_second_x, body_y},
        .change_callback = shirt_colour_change_callback,
    };
    body_y += body_increment_y;
    const MenuOption option_head_colour_change = {
        .name = "H-Colour",
        .max_index = MAX_COLOURS,
        .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
        .drawn_position = {body_second_x, body_y},
        .change_callback = head_colour_change_callback,
    };
    body_y += body_increment_y;
    const MenuOption option_accessory_colour_change = {
        .name = "A-Colour",
        .max_index = MAX_COLOURS,
        .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
        .drawn_position = {body_second_x, body_y},
        .change_callback = accessory_colour_change_callback,
    };
    body_y += body_increment_y * 1.5;
    const MenuOption option_next_change = {
        .name = "Next",
        .max_index = 0,
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {body_second_x, body_y},
        .change_callback = next_change_callback,
    };
    const MenuOption option_random_character = {
        .name = "Random",
        .max_index = 0,
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {body_x, body_y},
        .change_callback = random_character_callback,
    };
    body_y += body_increment_y;
    menu_tab_add_option(tab_character, &option_body_change, 0,             (Position){1, 0});
    menu_tab_add_option(tab_character, &option_shoes_change, 0,            (Position){0, 1});
    menu_tab_add_option(tab_character, &option_pants_change, 0,            (Position){0, 2});
    menu_tab_add_option(tab_character, &option_shirt_change, 0,            (Position){0, 3});
    menu_tab_add_option(tab_character, &option_head_change, 0,             (Position){0, 4});
    menu_tab_add_option(tab_character, &option_accessory_change, 0,        (Position){0, 5});
    menu_tab_add_option(tab_character, &option_character_creation_back, 0, (Position){0, 0});
    menu_tab_add_option(tab_character, &option_shoes_colour_change, 0,     (Position){1, 1});
    menu_tab_add_option(tab_character, &option_pants_colour_change, 0,     (Position){1, 2});
    menu_tab_add_option(tab_character, &option_shirt_colour_change, 0,     (Position){1, 3});
    menu_tab_add_option(tab_character, &option_head_colour_change, 0,      (Position){1, 4});
    menu_tab_add_option(tab_character, &option_accessory_colour_change, 0, (Position){1, 5});
    menu_tab_add_option(tab_character, &option_next_change, 0,             (Position){1, 6});
    menu_tab_add_option(tab_character, &option_random_character, 0,        (Position){0, 6});

    const MenuOption option_wits = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {60, 77},
            .change_callback = stats_wits_callback,
    };
    const MenuOption option_perception = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {47, 89},
            .change_callback = stats_perception_callback,
    };
    const MenuOption option_escape = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {62, 97},
            .change_callback = stats_escape_callback,
    };
    const MenuOption option_deception = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {51, 105},
            .change_callback = stats_deception_callback,
    };
    const MenuOption option_trapping = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {54, 113},
            .change_callback = stats_trapping_callback,
    };
    const MenuOption option_martial = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {34, 141},
            .change_callback = stats_martial_callback,
    };
    const MenuOption option_blades = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {65, 153},
            .change_callback = stats_blades_callback,
    };
    const MenuOption option_marksman = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {49, 161},
            .change_callback = stats_marksman_callback,
    };
    const MenuOption option_grappler = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {60, 169},
            .change_callback = stats_grappler_callback,
    };
    const MenuOption option_evade = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {66, 176},
            .change_callback = stats_evade_callback,
    };
    const MenuOption option_grit = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {250, 78},
            .change_callback = stats_grit_callback,
    };
    const MenuOption option_suffer = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {246, 89},
            .change_callback = stats_suffer_callback,
    };
    const MenuOption option_healing = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {250, 97},
            .change_callback = stats_healing_callback,
    };
    const MenuOption option_deaths_door = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {274, 105},
            .change_callback = stats_deaths_door_callback,
    };
    const MenuOption option_attrition = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {253, 113},
            .change_callback = stats_attrition_callback,
    };
    const MenuOption option_learning = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {286, 141},
            .change_callback = stats_learning_callback,
    };
    const MenuOption option_occult = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {247, 153},
            .change_callback = stats_occult_callback,
    };
    const MenuOption option_herbalism = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {261, 161},
            .change_callback = stats_herbalism_callback,
    };
    const MenuOption option_tactics = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {251, 169},
            .change_callback = stats_tactics_callback,
    };
    const MenuOption option_cartography = {
            .name = "",
            .max_index = 0,
            .type = MENU_OPTION_TYPE_CYCLE_INFINITE,
            .drawn_position = {272, 176},
            .change_callback = stats_cartography_callback,
    };
    menu_tab_add_option(tab_stats, &option_wits, 0,       (Position){0, 0});
    menu_tab_add_option(tab_stats, &option_perception, 0, (Position){0, 1});
    menu_tab_add_option(tab_stats, &option_escape, 0,     (Position){0, 2});
    menu_tab_add_option(tab_stats, &option_deception, 0,  (Position){0, 3});
    menu_tab_add_option(tab_stats, &option_trapping, 0,   (Position){0, 4});
    menu_tab_add_option(tab_stats, &option_martial, 0,       (Position){0, 5});
    menu_tab_add_option(tab_stats, &option_blades, 0, (Position){0, 6});
    menu_tab_add_option(tab_stats, &option_marksman, 0,     (Position){0, 7});
    menu_tab_add_option(tab_stats, &option_grappler, 0,  (Position){0, 8});
    menu_tab_add_option(tab_stats, &option_evade, 0,   (Position){0, 9});
    menu_tab_add_option(tab_stats, &option_grit, 0,       (Position){1, 0});
    menu_tab_add_option(tab_stats, &option_suffer, 0, (Position){1, 1});
    menu_tab_add_option(tab_stats, &option_healing, 0,     (Position){1, 2});
    menu_tab_add_option(tab_stats, &option_deaths_door, 0,  (Position){1, 3});
    menu_tab_add_option(tab_stats, &option_attrition, 0,   (Position){1, 4});
    menu_tab_add_option(tab_stats, &option_learning, 0,       (Position){1, 5});
    menu_tab_add_option(tab_stats, &option_occult, 0, (Position){1, 6});
    menu_tab_add_option(tab_stats, &option_herbalism, 0,     (Position){1, 7});
    menu_tab_add_option(tab_stats, &option_tactics, 0,  (Position){1, 8});
    menu_tab_add_option(tab_stats, &option_cartography, 0,   (Position){1, 9});
}

static void draw_pips_big(Oct_Vec2 position, float direction, int32_t count) {
    const float y = position[1];
    const float pip_horizontal_jump = direction * 6;
    const Oct_Asset pip_tex = oct_GetAsset(g_game.assets, "hud/skillpip.png");
    float x = position[0];
    Oct_Colour base_colour = {.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f};
    for (int32_t i = 0; i < count; i++) {
        Oct_Colour *c = &base_colour;
        oct_DrawTextureColour(
                pip_tex,
                c,
                (Oct_Vec2){x, y});
        x += pip_horizontal_jump;
    }
}

static void draw_pips(Oct_Vec2 position, float direction, int32_t count) {
    const float y = position[1];
    const float pip_horizontal_jump = direction * 4;
    const Oct_Asset pip_tex = oct_GetAsset(g_game.assets, "hud/ingamepip.png");
    float x = position[0];
    Oct_Colour base_colour = {.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f};
    for (int32_t i = 0; i < count; i++) {
        Oct_Colour *c = &base_colour;
        oct_DrawTextureColour(
                pip_tex,
                c,
                (Oct_Vec2){x, y});
        x += pip_horizontal_jump;
    }
}

void menu_update() {
    oct_LockCameras(g_game.ui_camera);
    oct_SetTextureCamerasEnabled(false);

    // Draw some title stuff
    oct_DrawTexture(
        oct_GetAsset(g_game.assets, "menu/title.png"),
        (Oct_Vec2){(320 - 298) / 2, 10});

    // Menu system
    menu_system_process_and_draw(&test_menu);

    // Draw the player's preview if we are customizing their appearence atm
    if (menu_get_current_tab(&test_menu) == tab_index_character) {
        draw_object(
            &g_game.player.info,
            (Oct_Vec2){(VIRTUAL_WIDTH / 2) - 10, (VIRTUAL_HEIGHT / 2)},
            4,
            1);
    } else if (menu_get_current_tab(&test_menu) == tab_index_stats) {
        oct_DrawTexture(oct_GetAsset(g_game.assets, "menu/statsscreen.png"), (Oct_Vec2){0, 0});

        // Draw pips in each skill
        draw_pips_big((Oct_Vec2){92, 74}, 1, player_starting_statblock.wits);
        draw_pips((Oct_Vec2){92, 89 + (0 * 8)}, 1, player_starting_statblock.wits_stats[0]);
        draw_pips((Oct_Vec2){92, 89 + (1 * 8)}, 1, player_starting_statblock.wits_stats[1]);
        draw_pips((Oct_Vec2){92, 89 + (2 * 8)}, 1, player_starting_statblock.wits_stats[2]);
        draw_pips((Oct_Vec2){92, 89 + (3 * 8)}, 1, player_starting_statblock.wits_stats[3]);

        draw_pips_big((Oct_Vec2){92, 138}, 1, player_starting_statblock.martial);
        draw_pips((Oct_Vec2){92, 153 + (0 * 8)}, 1, player_starting_statblock.martial_stats[0]);
        draw_pips((Oct_Vec2){92, 153 + (1 * 8)}, 1, player_starting_statblock.martial_stats[1]);
        draw_pips((Oct_Vec2){92, 153 + (2 * 8)}, 1, player_starting_statblock.martial_stats[2]);
        draw_pips((Oct_Vec2){92, 153 + (3 * 8)}, 1, player_starting_statblock.martial_stats[3]);

        const float lsx = 215 - 4;
        const float small_offset = 2;
        draw_pips_big((Oct_Vec2){lsx, 74}, -1, player_starting_statblock.grit);
        draw_pips((Oct_Vec2){lsx + small_offset, 89 + (0 * 8)}, -1, player_starting_statblock.grit_stats[0]);
        draw_pips((Oct_Vec2){lsx + small_offset, 89 + (1 * 8)}, -1, player_starting_statblock.grit_stats[1]);
        draw_pips((Oct_Vec2){lsx + small_offset, 89 + (2 * 8)}, -1, player_starting_statblock.grit_stats[2]);
        draw_pips((Oct_Vec2){lsx + small_offset, 89 + (3 * 8)}, -1, player_starting_statblock.grit_stats[3]);

        draw_pips_big((Oct_Vec2){lsx, 138}, -1, player_starting_statblock.learning);
        draw_pips((Oct_Vec2){lsx + small_offset, 153 + (0 * 8)}, -1, player_starting_statblock.learning_stats[0]);
        draw_pips((Oct_Vec2){lsx + small_offset, 153 + (1 * 8)}, -1, player_starting_statblock.learning_stats[1]);
        draw_pips((Oct_Vec2){lsx + small_offset, 153 + (2 * 8)}, -1, player_starting_statblock.learning_stats[2]);
        draw_pips((Oct_Vec2){lsx + small_offset, 153 + (3 * 8)}, -1, player_starting_statblock.learning_stats[3]);

        // Draw the remaining skill and base points
        Oct_Vec2 size = {0};
        Oct_FontAtlas font = oct_GetAsset(g_game.assets, "fnt_pixel");
        oct_GetTextSize(font, size, 1, "Skill Points: %i / Base Points: %i", player_available_skill_points, player_available_base_points);
        oct_DrawText(font, (Oct_Vec2){roundf((VIRTUAL_WIDTH / 2) - (size[0] / 2)), 50}, 1, "Skill Points: %i / Base Points: %i", player_available_skill_points, player_available_base_points);
    }

    if (should_play_game && !in_level_transition()) {
        queue_level_transition(LEVEL_INDEX_FLOOR_1, TRANSITION_TYPE_FADE_OUT, 30);
    }
}

void menu_end() {
    menu_system_destroy(&test_menu);
}
