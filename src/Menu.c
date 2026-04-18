#include <oct/Octarine.h>
#include "Game.h"
#include "MenuSystem.h"

static MenuSystem test_menu = {0};
static const int32_t TAB_COUNT = 3;
static MenuSystemTab *tab_start;
static const int32_t tab_index_start = 0;
static MenuSystemTab *tab_options;
static const int32_t tab_index_options = 1;
static MenuSystemTab *tab_character;
static const int32_t tab_index_character = 2;
static bool should_play_game = false;

static void play_change_callback(int32_t _) {
    debug("Pressed play");
    should_play_game = true;
}
static void cycler_change_callback(int32_t index) {
    debug("Changed cycler to %i", index);
}
static void exit_change_callback(int32_t _) {
    debug("Pressed exit");
}

void menu_begin() {
    menu_system_initialize(&test_menu, 3);
    tab_start = menu_get_tab(&test_menu, tab_index_start);
    tab_options = menu_get_tab(&test_menu, tab_index_options);
    tab_character = menu_get_tab(&test_menu, tab_index_character);
    MenuOption option_play = {
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {10, 10},
        .name = "Play",
        .draw_callback = nullptr,
        .change_callback = play_change_callback,
    };
    MenuOption option_cycle = {
        .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
        .drawn_position = {10, 40},
        .name = "Cycle",
        .index = 0,
        .max_index = 3,
        .draw_callback = nullptr,
        .change_callback = cycler_change_callback,
    };
    MenuOption option_exit = {
        .type = MENU_OPTION_TYPE_SELECT,
        .drawn_position = {10, 70},
        .name = "Exit",
        .draw_callback = nullptr,
        .change_callback = exit_change_callback,
    };
    menu_tab_add_option(tab_start, &option_play, (Position){0, 0});
    menu_tab_add_option(tab_start, &option_cycle, (Position){0, 1});
    menu_tab_add_option(tab_start, &option_exit, (Position){0, 2});
}

LevelIndex menu_update() {
    menu_system_process_and_draw(&test_menu);
    return LEVEL_INDEX_FLOOR_1;//should_play_game ? LEVEL_INDEX_FLOOR_1 : LEVEL_INDEX_MENU;
}

void menu_end() {
    menu_system_destroy(&test_menu);
}
