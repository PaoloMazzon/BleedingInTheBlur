#include <string.h>
#include <assert.h>
#include "Game.h"
#include "MenuSystem.h"

const int32_t INVALID_GRID_POS = -1;

// Returns the menu option at a given grid position or nullptr if that position doesnt have an option
static MenuOption *get_menu_grid_pos(MenuTab *tab, Position p) {
    assert(tab);
    if (p[0] >= MAX_MENU_WIDTH || p[1] >= MAX_MENU_HEIGHT || p[0] < 0 || p[1] < 0)
        return nullptr;
    const int32_t value_at_pos = tab->menu_grid[(p[1] * MAX_MENU_HEIGHT) + p[0]];
    if (value_at_pos != INVALID_GRID_POS)
        return &tab->menu_options[value_at_pos];
    return nullptr;
}

void menu_system_initialize(MenuSystem *menu_system, int32_t tab_count) {
    memset(menu_system, 0, sizeof(MenuSystem));
    menu_system->tabs = oct_Zalloc(g_game.allocator, sizeof(MenuTab) * tab_count);
}

void menu_system_destroy(MenuSystem *menu_system) {
    oct_Free(g_game.allocator, menu_system->tabs);
}

MenuTab *menu_get_tab(MenuSystem *system, int32_t index) {
    assert(system);
    assert(index < system->tab_count && index >= 0);
    return &system->tabs[index];
}

void menu_tab_add_option(MenuTab *tab, const MenuOption *new_option, const Position new_pos_in_menu) {
    assert(tab);
    assert(new_option);
    assert(new_pos_in_menu[0] < MAX_MENU_WIDTH && new_pos_in_menu[1] < MAX_MENU_HEIGHT && new_pos_in_menu[0] >= 0 && new_pos_in_menu[1] >= 0);
    const int32_t index = tab->menu_option_count;
    tab->menu_option_count += 1;
    assert(index != MAX_MENU_ELEMENTS);
    memcpy(&tab->menu_options[index], new_option, sizeof(MenuOption));
    tab->menu_grid[(new_pos_in_menu[1] * MAX_MENU_HEIGHT) + new_pos_in_menu[0]] = index;
}

void menu_system_process_and_draw(MenuSystem *system) {
    // TODO: This
}
