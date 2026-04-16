#include <string.h>
#include <assert.h>
#include "MenuSystem.h"

const int32_t INVALID_GRID_POS = -1;

// Returns the menu option at a given grid position or nullptr if that position doesnt have an option
static MenuOption *get_menu_grid_pos(MenuSystem *system, Position p) {
    assert(system);
    if (p[0] >= MAX_MENU_WIDTH || p[1] >= MAX_MENU_HEIGHT || p[0] < 0 || p[1] < 0)
        return nullptr;
    const int32_t value_at_pos = system->menu_grid[(p[1] * MAX_MENU_HEIGHT) + p[0]];
    if (value_at_pos != INVALID_GRID_POS)
        return &system->menu_options[value_at_pos];
    return nullptr;
}

void menu_system_initialize(MenuSystem *menu_system) {
    memset(menu_system, 0, sizeof(MenuSystem));
    for (int32_t i = 0; i < MAX_MENU_ELEMENTS; i++)
        menu_system->menu_grid[i] = INVALID_GRID_POS;
}

void menu_system_add_option(MenuSystem *system, const MenuOption *new_option, const Position new_pos_in_menu) {
    assert(system);
    assert(new_option);
    assert(new_pos_in_menu[0] < MAX_MENU_WIDTH && new_pos_in_menu[1] < MAX_MENU_HEIGHT && new_pos_in_menu[0] >= 0 && new_pos_in_menu[1] >= 0);
    const int32_t index = system->menu_option_count;
    system->menu_option_count += 1;
    assert(index != MAX_MENU_ELEMENTS);
    memcpy(&system->menu_options[index], new_option, sizeof(MenuOption));
    system->menu_grid[(new_pos_in_menu[1] * MAX_MENU_HEIGHT) + new_pos_in_menu[0]] = index;
}

void menu_system_process_and_draw(MenuSystem *system) {
    // TODO: This
}
