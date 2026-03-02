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
void create_label(const char *text, Position position, Oct_Colour colour, bool needs_to_be_freed);
Character *level_get_character_slot();
// this can return null if its out of bounds
TileContents *level_get_tile(int32_t x, int32_t y);

void *startup();
void *update(void *ptr);
void shutdown(void *ptr);

void create_slime(Character *slot);