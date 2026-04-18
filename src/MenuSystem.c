#include <string.h>
#include <assert.h>
#include "Game.h"
#include "MenuSystem.h"

const int32_t INVALID_GRID_POS = -1;

// Returns the menu option at a given grid position or nullptr if that position doesnt have an option
static MenuOption *get_menu_grid_pos(MenuSystemTab *tab, Position p) {
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

MenuSystemTab *menu_get_tab(MenuSystem *system, int32_t index) {
    assert(system);
    assert(index < system->tab_count && index >= 0);
    return &system->tabs[index];
}

void menu_set_tab(MenuSystem *system, int32_t index) {
    assert(system);
    assert(index < system->tab_count && index >= 0);
    system->current_tab = index;
}

void menu_tab_add_option(MenuSystemTab *tab, const MenuOption *new_option, const Position new_pos_in_menu) {
    assert(tab);
    assert(new_option);
    assert(new_pos_in_menu[0] < MAX_MENU_WIDTH && new_pos_in_menu[1] < MAX_MENU_HEIGHT && new_pos_in_menu[0] >= 0 && new_pos_in_menu[1] >= 0);
    const int32_t index = tab->menu_option_count;
    tab->menu_option_count += 1;
    assert(index != MAX_MENU_ELEMENTS);
    memcpy(&tab->menu_options[index], new_option, sizeof(MenuOption));
    tab->menu_grid[(new_pos_in_menu[1] * MAX_MENU_HEIGHT) + new_pos_in_menu[0]] = index;
}

// Returns the next index this should go to (y value should be -1 or 1). Can return the same index if there is no other spot
static int32_t next_vertical_spot(MenuSystemTab *tab, int32_t x_pos, int32_t move) {
    int32_t new_index = tab->cursor_pos[1] + move;
    MenuOption *new_option = get_menu_grid_pos(tab, (Position){x_pos, new_index});
    const int32_t max_iterations = 20;
    int32_t iterations = 0;
    while (!new_option) {
        new_index += move;
        if (new_index > MAX_MENU_HEIGHT)
            new_index = 0;
        if (new_index < 0)
            new_index = MAX_MENU_HEIGHT - 1;
        new_option = get_menu_grid_pos(tab, (Position){x_pos, new_index});
        if (iterations++ >= max_iterations)
            oct_Raise(OCT_STATUS_ERROR, true, "Menu was setup incorrectly");
    }
    return new_index;
}

// Returns the next index this should go to (y value should be -1 or 1). Can return the same index if there is no other spot
static int32_t next_horizontal_spot(MenuSystemTab *tab, int32_t y_pos, int32_t move) {
    int32_t new_index = tab->cursor_pos[1] + move;
    MenuOption *new_option = get_menu_grid_pos(tab, (Position){new_index, y_pos});
    const int32_t max_iterations = 20;
    int32_t iterations = 0;
    while (!new_option) {
        new_index += move;
        if (new_index > MAX_MENU_WIDTH)
            new_index = 0;
        if (new_index < 0)
            new_index = MAX_MENU_WIDTH - 1;
        new_option = get_menu_grid_pos(tab, (Position){new_index, y_pos});
        if (iterations++ >= max_iterations)
            oct_Raise(OCT_STATUS_ERROR, true, "Menu was setup incorrectly");
    }
    return new_index;
}

void menu_system_process_and_draw(MenuSystem *system) {
    const int32_t move_horizontal = (int32_t)oct_KeyPressed(BUTTON_RIGHT) - (int32_t)oct_KeyPressed(BUTTON_LEFT);
    const int32_t move_vertical = (int32_t)oct_KeyPressed(BUTTON_DOWN) - (int32_t)oct_KeyPressed(BUTTON_UP);
    MenuSystemTab *tab = &system->tabs[system->current_tab];

    // TODO: Add a check to see if we are cycling an option or changing options
    if (move_horizontal != 0)
        tab->cursor_pos[0] = next_horizontal_spot(tab, tab->cursor_pos[1], move_horizontal);
    if (move_vertical != 0)
        tab->cursor_pos[1] = next_vertical_spot(tab, tab->cursor_pos[0], move_vertical);

    if (oct_KeyPressed(BUTTON_CONFIRM)) {
        MenuOption *option = get_menu_grid_pos(tab, tab->cursor_pos);
        assert(option); // if this fails then next_x_spot is failing probably

        // TODO: Either toggle lock the cursor to this option if its cycling or select it if its not cycling
    }

    // TODO: Draw all options
}
