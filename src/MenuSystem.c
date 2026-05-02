#include <string.h>
#include <assert.h>
#include "Game.h"
#include "MenuSystem.h"
#include "Util.h"

#define sign(x) (x > 0.0f ? 1.0f : -1.0f)

const int32_t INVALID_GRID_POS = -1;

// Returns the menu option at a given grid position or nullptr if that position doesnt have an option
static MenuOption *get_menu_grid_pos(MenuSystemTab *tab, Position p) {
    assert(tab);
    if (p[0] >= MAX_MENU_WIDTH || p[1] >= MAX_MENU_HEIGHT || p[0] < 0 || p[1] < 0)
        return nullptr;
    const int32_t value_at_pos = tab->menu_grid[(p[1] * MAX_MENU_WIDTH) + p[0]];
    if (value_at_pos != INVALID_GRID_POS)
        return &tab->menu_options[value_at_pos];
    return nullptr;
}

void menu_system_initialize(MenuSystem *menu_system, int32_t tab_count) {
    memset(menu_system, 0, sizeof(MenuSystem));
    menu_system->tabs = oct_Zalloc(g_game.allocator, sizeof(MenuSystemTab) * tab_count);
    menu_system->tab_count = tab_count;
    for (int32_t i = 0; i < tab_count; i++)
        for (int32_t j = 0; j < MAX_MENU_ELEMENTS; j++)
            menu_system->tabs[i].menu_grid[j] = INVALID_GRID_POS;
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
    system->tabs[system->current_tab].selected_current_option = false;
    system->current_tab = index;
}

void menu_tab_add_option(MenuSystemTab *tab, const MenuOption *new_option, int32_t default_index, const Position new_pos_in_menu) {
    assert(tab);
    assert(new_option);
    assert(new_pos_in_menu[0] < MAX_MENU_WIDTH && new_pos_in_menu[1] < MAX_MENU_HEIGHT && new_pos_in_menu[0] >= 0 && new_pos_in_menu[1] >= 0);
    const int32_t index = tab->menu_option_count;
    tab->menu_option_count += 1;
    assert(index != MAX_MENU_ELEMENTS);
    memcpy(&tab->menu_options[index], new_option, sizeof(MenuOption));
    tab->menu_options[index].tween_position[0] = tab->menu_options[index].drawn_position[0];
    tab->menu_options[index].tween_position[1] = tab->menu_options[index].drawn_position[1];
    tab->menu_grid[(new_pos_in_menu[1] * MAX_MENU_WIDTH) + new_pos_in_menu[0]] = index;
    tab->menu_options[index].id = new_oct_id();
    tab->menu_options[index].index = default_index;
}

// Returns the next index this should go to (y value should be -1 or 1). Can return the same index if there is no other spot
static int32_t next_vertical_spot(MenuSystemTab *tab, int32_t x_pos, int32_t move) {
    // TODO: This should move the cursor horizontal as well in the event of
    //       x  x  x <-- here
    //       x <-- going down should go here
    //       x  x  x  x
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
    int32_t new_index = tab->cursor_pos[0] + move;
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
    MenuOption *option = get_menu_grid_pos(tab, tab->cursor_pos);
    assert(option); // if this fails then next_x_spot is failing probably

    if (!tab->selected_current_option) {
        if (move_horizontal != 0)
            tab->cursor_pos[0] = next_horizontal_spot(tab, tab->cursor_pos[1], move_horizontal);
        if (move_vertical != 0)
            tab->cursor_pos[1] = next_vertical_spot(tab, tab->cursor_pos[0], move_vertical);
    } else if (move_horizontal != 0) {
        // Cycle current option
        if (option->type == MENU_OPTION_TYPE_CYCLE_HORIZONTAL && move_horizontal != 0) {
            option->index += move_horizontal;

            if (option->index >= option->max_index)
                option->index = 0;
            if (option->index < 0)
                option->index = option->max_index - 1;

            option->change_callback(option->index);
            option->bounce_amount = move_horizontal * 3;
        }
        if (option->type == MENU_OPTION_TYPE_CYCLE_INFINITE && move_horizontal != 0) {
            option->change_callback(move_horizontal);
            option->bounce_amount = move_horizontal * 3;
        }
    }

    if (oct_KeyPressed(BUTTON_CONFIRM) || oct_KeyPressed(OCT_KEY_SPACE) || oct_KeyPressed(OCT_KEY_RETURN)) {
        // Start cycling
        if (!tab->selected_current_option && option->type != MENU_OPTION_TYPE_SELECT) {
            tab->selected_current_option = true;
        } else if (tab->selected_current_option) { // un cycle
            tab->selected_current_option = false;
        } else { // choose this option
            option->change_callback(option->index);
        }
    }

    for (int32_t i = 0; i < tab->menu_option_count; i++) {
        MenuOption *current_option = &tab->menu_options[i];
        const bool selected = option == current_option;
        const float select_offset = 8;

        // Find the target position for this piece of text then tween to it
        const Oct_Vec2 target_pos = {
            current_option->drawn_position[0] + (selected ? select_offset : 0),
            current_option->drawn_position[1]
        };
        current_option->tween_position[0] += (target_pos[0] - current_option->tween_position[0]) * 0.4f;
        current_option->tween_position[1] += (target_pos[1] - current_option->tween_position[1]) * 0.4f;

        // Draw the option text
        oct_DrawTextInt(
            OCT_INTERPOLATE_ALL, current_option->id,
            oct_GetAsset(g_game.assets, "fnt_pixel"),
            (Oct_Vec2){current_option->tween_position[0] + current_option->bounce_amount, current_option->tween_position[1]}, 1,
            "%s", current_option->name);

        // Tweening bounce logic
        current_option->bounce_amount -= current_option->bounce_amount * 0.95; // todo proper oscillation

        // Draw arrows in the event that the option is a cycling one
        if (current_option->type != MENU_OPTION_TYPE_SELECT && current_option == option && tab->selected_current_option) {
            Oct_Vec2 text_size;
            oct_GetTextSize(
                oct_GetAsset(g_game.assets, "fnt_pixel"),
                text_size, 1,
                "%s", current_option->name);
            oct_DrawTexture(
                    oct_GetAsset(g_game.assets, "menu/arrow_left.png"),
                    (Oct_Vec2){current_option->tween_position[0] - 12, current_option->tween_position[1] + 1});
            oct_DrawTexture(
                    oct_GetAsset(g_game.assets, "menu/arrow_right.png"),
                    (Oct_Vec2){current_option->tween_position[0] + text_size[0] + 3, current_option->tween_position[1] + 1});
        } else if (current_option == option) {
            oct_DrawTexture(
                    oct_GetAsset(g_game.assets, "menu/pointer.png"),
                    (Oct_Vec2){current_option->tween_position[0] - 20, current_option->tween_position[1] + 1});
        }

        // And the optional drawing callback
        if (current_option->draw_callback) {
            current_option->draw_callback(current_option->drawn_position, current_option->index);
        }
    }
}
