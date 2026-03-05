/// \brief Declares all game-related functions
#pragma once
#include "Structs.h"

// Global game state
extern Game g_game;

void menu_begin();
LevelIndex menu_update(); // return value is where to go
void menu_end();

void level_begin();
LevelIndex level_update(); // return value is where to go
void level_end();
void create_label(const char *text, const Position position, Oct_Colour colour, bool needs_to_be_freed);
void create_dice_label(const char *text, const Position position, Oct_Colour colour, bool needs_to_be_freed);
Character *level_get_character_slot();
// this can return null if it's out of bounds
TileContents *level_get_tile(int32_t x, int32_t y);
TileContentsType level_get_tile_type(int32_t x, int32_t y); // returns TILE_CONTENTS_TYPE_NONE if out of bounds

// Returns true if there is anything in the popup stack
bool popups_are_active();
void draw_and_update_popups();

// Popups are asynchronous "windows" whose output values need to be queried with their pointer
void popup_message(const char *text, bool needs_to_be_freed);
PopupInputPointer popup_input(const char *text, bool needs_to_be_freed);
PopupDicePointer popup_dice(int32_t dc, int32_t pips);

// Returns false if the popup is not yet resolved, true otherwise
bool popup_get_input(PopupInputPointer input_pointer, const char *out);
bool popup_get_dice(PopupDicePointer dice_pointer, int32_t *out);

void player_init();
void player_update();

void *startup();
void *update(void *ptr);
void shutdown(void *ptr);

void create_slime(Character *slot, Position pos);