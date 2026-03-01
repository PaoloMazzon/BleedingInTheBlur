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

void *startup();
void *update(void *ptr);
void shutdown(void *ptr);