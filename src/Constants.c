#include "Structs.h"

const char *BASE_STAT_NAMES[4] = {
        "Grit",
        "Learning",
        "Wits",
        "Martial"
};

const char *GRIT_STAT_NAMES[4] = {
        "Suffer",
        "Healing",
        "Death's Door",
        "Attrition"
};

const char *LEARNING_STAT_NAMES[4] = {
        "Occult",
        "Herbalism",
        "Tactics",
        "Cartography"
};

const char *WITS_STAT_NAMES[4] = {
        "Perception",
        "Escape",
        "Deception",
        "Trapping"
};

const char *MARTIAL_STAT_NAMES[4] = {
        "Blades",
        "Marksman",
        "Grappler",
        "Evade"
};
extern const char *WEAPON_TYPE_NAMES[6] = {
        "Sword",
        "Spear",
        "Bow",
        "Crossbow",
        "Dagger",
        "Other",
};

extern const char *RARITY_NAMES[4] = {
        "Common",
        "Uncommon",
        "Rare",
        "Mythic",
};

const int32_t SOUL_BOUND_WEAPON_SLOT = 1;
const int32_t STARTING_WEAPON_SLOT = 0;
const int32_t STARTING_BASE_STAT_POINTS = 15;
const int32_t BASE_STAT_MAX = 10;
const int32_t STARTING_SKILL_PIPS = 22;
const float CELL_WIDTH = 8;
const float CELL_HEIGHT = 8;
const float VIRTUAL_WIDTH = 320;
const float VIRTUAL_HEIGHT = 256;
const float GAME_VIEW_WIDTH = 160;
const float GAME_VIEW_HEIGHT = 128;

const Oct_Key BUTTON_ATTACK_VIEW = OCT_KEY_X;
const Oct_Key BUTTON_STAT_TOGGLE = OCT_KEY_C;
const Oct_Key BUTTON_LEFT = OCT_KEY_LEFT;
const Oct_Key BUTTON_RIGHT = OCT_KEY_RIGHT;
const Oct_Key BUTTON_UP = OCT_KEY_UP;
const Oct_Key BUTTON_DOWN = OCT_KEY_DOWN;
