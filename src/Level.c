#include <oct/Octarine.h>
#include "Game.h"
#include "Character.h"

void level_begin() {
    Statblock sb;
    random_statblock(&sb);
    print_statblock(&sb);
    character_create(&sb, &g_game.player);
}

LevelIndex level_update() {
    return g_game.level_index;
}

void level_end() {

}
