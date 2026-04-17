/// \brief Menuing system for the main menu and in game
#pragma once
#include "Structs.h"

// Zeroes a memory system so its ready to be used (like at the start of a level or whatever)
void menu_system_initialize(MenuSystem *menu_system, int32_t tab_count);
void menu_system_destroy(MenuSystem *menu_system);

// Copies an option into an existing menu tab at a given position
void menu_tab_add_option(MenuTab *tab, const MenuOption *new_option, const Position new_pos_in_menu);

// Returns the tab at that position. Must be less than the amount specified when you created the system
MenuTab *menu_get_tab(MenuSystem *system, int32_t index);

// Updates and draws a menu system
void menu_system_process_and_draw(MenuSystem *system);
