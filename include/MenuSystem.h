/// \brief Menuing system for the main menu and in game
#pragma once
#include "Structs.h"

// Zeroes a memory system so its ready to be used (like at the start of a level or whatever)
void menu_system_initialize(MenuSystem *menu_system);

// Copies an option into an existing menu system at a given position
void menu_system_add_option(MenuSystem *system, const MenuOption *new_option, const Position new_pos_in_menu);

// Updates and draws a menu system
void menu_system_process_and_draw(MenuSystem *system);
