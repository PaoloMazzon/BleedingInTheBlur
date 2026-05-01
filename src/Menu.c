#include <string.h>
#include <oct/Octarine.h>
#include "Game.h"
#include "MenuSystem.h"
#include "MenuDetails.h"

static MenuSystem test_menu = {0};
static const int32_t TAB_COUNT = 3;
static MenuSystemTab *tab_start;
static const int32_t tab_index_start = 0;
static MenuSystemTab *tab_options;
static const int32_t tab_index_options = 1;
static MenuSystemTab *tab_character;
static const int32_t tab_index_character = 2;
static bool should_play_game = false;

static Statblock player_starting_statblock;
static int32_t player_available_points;

static void play_change_callback(int32_t _) {
    debug("Pressed play");
    should_play_game = true;
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

void menu_begin() {
    memset(&player_starting_statblock, 0, sizeof(Statblock));
    player_available_points = 20;

    menu_system_initialize(&test_menu, 3);
    tab_start = menu_get_tab(&test_menu, tab_index_start);
    tab_options = menu_get_tab(&test_menu, tab_index_options);
    tab_character = menu_get_tab(&test_menu, tab_index_character);
    MenuOption option_play = {
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {140, 175},
        .name = "Play",
        .draw_callback = nullptr,
        .change_callback = play_change_callback,
    };
    MenuOption option_cycle = {
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {140, 195},
        .name = "Options",
        .draw_callback = nullptr,
        .change_callback = settings_change_callback,
    };
    MenuOption option_exit = {
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {140, 215},
        .name = "Exit",
        .draw_callback = nullptr,
        .change_callback = exit_change_callback,
    };
    MenuOption option_back = {
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {140, 215},
        .name = "Back",
        .draw_callback = nullptr,
        .change_callback = back_change_callback,
    };
    menu_tab_add_option(tab_start, &option_play, (Position){0, 0});
    menu_tab_add_option(tab_start, &option_cycle, (Position){0, 1});
    menu_tab_add_option(tab_start, &option_exit, (Position){0, 2});
    menu_tab_add_option(tab_options, &music_volume_option,           (Position){0, 0});
    menu_tab_add_option(tab_options, &sfx_volume_option,             (Position){0, 1});
    menu_tab_add_option(tab_options, &animation_speed_option,        (Position){0, 2});
    menu_tab_add_option(tab_options, &auto_pick_up_item_option,      (Position){0, 3});
    menu_tab_add_option(tab_options, &animate_enemy_movement_option, (Position){0, 4});
    menu_tab_add_option(tab_options, &fullscreen_option,             (Position){0, 5});
    menu_tab_add_option(tab_options, &scale_mode_option,             (Position){0, 6});
    menu_tab_add_option(tab_options, &option_back,                   (Position){0, 7});
}

LevelIndex menu_update() {
    oct_LockCameras(g_game.ui_camera);
    oct_SetTextureCamerasEnabled(false);

    // Draw some title stuff
    oct_DrawTexture(
        oct_GetAsset(g_game.assets, "menu/title.png"),
        (Oct_Vec2){(320 - 298) / 2, 10});

    // Menu system
    menu_system_process_and_draw(&test_menu);

    return should_play_game ? LEVEL_INDEX_FLOOR_1 : LEVEL_INDEX_MENU;
}

void menu_end() {
    menu_system_destroy(&test_menu);
}
