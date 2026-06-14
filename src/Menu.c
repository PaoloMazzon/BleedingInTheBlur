#include <string.h>
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
static const int32_t tab_index_stats = 2;
static bool should_play_game = false;

static Statblock player_starting_statblock;
static int32_t player_available_points;

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



/********************************* Actual menu logic *********************************/
void menu_begin() {
    memset(&player_starting_statblock, 0, sizeof(Statblock));
    player_available_points = 20;

    // Setup the player to be able to be drawn without properly initializing the full character
    info_set_random_sprite_layers(&g_game.player.info);
    g_game.player.info.scale_x = 1;

    menu_system_initialize(&test_menu, 3);
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
    }

    if (should_play_game && !in_level_transition()) {
        queue_level_transition(LEVEL_INDEX_FLOOR_1, TRANSITION_TYPE_FADE_OUT, 30);
    }
}

void menu_end() {
    menu_system_destroy(&test_menu);
}
