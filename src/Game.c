#include <stddef.h>
#include <stdlib.h>
#include "Game.h"
#include "Structs.h"

Game g_game;

void *startup() {
    oct_Log("Starting the game.");
    menu_begin();
    return nullptr;
}

void *update(void *ptr) {
    if (g_game.level_index == LEVEL_INDEX_MENU) {
        const LevelIndex new_index = menu_update();
        if (new_index == LEVEL_INDEX_QUIT) {
            oct_Log("Quitting from the menu.");
            menu_end();
            abort();
        } else if (new_index != LEVEL_INDEX_MENU) {
            oct_Log("Switching from the menu to level index %i.", new_index);
            menu_end();
            g_game.level_index = new_index;
            level_begin();
        }
    } else {
        const LevelIndex new_index = level_update();
        if (new_index == LEVEL_INDEX_MENU) {
            oct_Log("Quitting from level %i to the menu.", g_game.level_index);
            level_end();
            g_game.level_index = new_index;
            menu_begin();
        } else if (new_index == LEVEL_INDEX_QUIT) {
            oct_Log("Quitting from level %i.", g_game.level_index);
            level_end();
            abort();
        } else if (new_index != g_game.level_index) {
            oct_Log("Going from level %i to level %i.", g_game.level_index, new_index);
            level_end();
            g_game.level_index = new_index;
            level_begin();
        }
    }
    return nullptr;
}

void shutdown(void *ptr) {

}
