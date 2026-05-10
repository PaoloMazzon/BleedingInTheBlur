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
const char *WEAPON_TYPE_NAMES[6] = {
        "Sword",
        "Spear",
        "Bow",
        "Crossbow",
        "Dagger",
        "Other",
};

const char *RARITY_NAMES[4] = {
        "Common",
        "Uncommon",
        "Rare",
        "Mythic",
};

const char *GAME_STATES[7] = {
        "Player Interaction",
        "Player Inventory",
        "Player Stats",
        "Player Spells",
        "Player Attack",
        "Enemy Turn",
        "Player Death",
};

extern const char *SPRITE_LAYER_NAMES[6] = {
        "Sprite Layer Body",
        "Sprite Layer Shoes",
        "Sprite Layer Pants",
        "Sprite Layer Shirt",
        "Sprite Layer Head",
        "Sprite Layer Accessories"
};

const char *SCALE_MODE_NAMES[3] = {
        "Integer",
        "Aspect Ratio",
        "Stretch",
};

const char *ANIMATION_SPEED_NAMES[4] = {
        "None",
        "Full",
        "Fast",
        "Faster",
};

const int32_t MAX_SPRITE_OPTIONS_PER_LAYER[6] = {
        10, // Body
        1, // Shoes
        2, // Pants
        4, // Shirt
        8, // Head
        10, // Accessory
};

const char *LEVEL_INDEX_NAMES[8] = {
        "Menu",
        "Floor 1",
        "Floor 2",
        "Floor 3",
        "Floor 4",
        "Floor 5",
        "Floor Final",
        "Quit",
};

const int32_t SOUL_BOUND_WEAPON_SLOT    = 1;
const int32_t STARTING_WEAPON_SLOT      = 0;
const int32_t STARTING_BASE_STAT_POINTS = 15;
const int32_t BASE_STAT_MAX             = 10;
const int32_t STARTING_SKILL_PIPS       = 22;
const int32_t TIMESTAMP_NOT_IN_USE      = 0;

const float CELL_WIDTH       = 8;
const float CELL_HEIGHT      = 8;
const float VIRTUAL_WIDTH    = 320;
const float VIRTUAL_HEIGHT   = 256;
const float GAME_VIEW_WIDTH  = 160;
const float GAME_VIEW_HEIGHT = 128;

const int32_t TILE_DOOR_CLOSED                   = 22 + 8;
const int32_t TILE_DOOR_OPEN                     = 23 + 8;
const int32_t TILE_FLOOR_SHADOW_LEFT             = 25 + 8;
const int32_t TILE_FLOOR_SHADOW_BELOW            = 28 + 8;
const int32_t TILE_FLOOR_SHADOW_CORNER           = 29 + 8;
const int32_t TILE_WALL_SHADOW_LEFT              = 27 + 8;
const int32_t TILE_WALL_SHADOW_RIGHT             = 25 + 8;
const int32_t TILE_WALL_SHADOW_ABOVE             = 28 + 8;
const int32_t TILE_WALL_SHADOW_BELOW             = 26 + 8;
const int32_t TILE_WALL_SHADOW_OUTER_CORNER_TR   = 29 + 8;
const int32_t TILE_WALL_SHADOW_OUTER_CORNER_TL   = 32 + 8;
const int32_t TILE_WALL_SHADOW_OUTER_CORNER_BR   = 30 + 8;
const int32_t TILE_WALL_SHADOW_OUTER_CORNER_BL   = 31 + 8;
const int32_t TILE_WALL_SHADOW_INNER_CORNER_TR   = 34 + 8;
const int32_t TILE_WALL_SHADOW_INNER_CORNER_TL   = 33 + 8;
const int32_t TILE_WALL_SHADOW_INNER_CORNER_BR   = 35 + 8;
const int32_t TILE_WALL_SHADOW_INNER_CORNER_BL   = 36 + 8;
const int32_t TILE_WALL_SHADOW_ABOVE_BELOW       = 37 + 8;
const int32_t TILE_WALL_SHADOW_ABOVE_BELOW_RIGHT = 40 + 8;
const int32_t TILE_WALL_SHADOW_ABOVE_BELOW_LEFT  = 39 + 8;
const int32_t TILE_WALL_SHADOW_LEFT_RIGHT        = 38 + 8;
const int32_t TILE_WALL_SHADOW_LEFT_RIGHT_TOP    = 41 + 8;
const int32_t TILE_WALL_SHADOW_LEFT_RIGHT_BOTTOM = 42 + 8;
const int32_t TILE_STAIRS_UP = 53;
const int32_t TILE_STAIRS_DOWN = 52;

const Oct_Key BUTTON_ATTACK_VIEW   = OCT_KEY_X;
const Oct_Key BUTTON_ITEM_SWAP     = OCT_KEY_C;
const Oct_Key BUTTON_ITEM_USE      = OCT_KEY_B;
const Oct_Key BUTTON_CONFIRM       = OCT_KEY_Z;
const Oct_Key BUTTON_SWAP_WEAPON   = OCT_KEY_V;
const Oct_Key BUTTON_MENU_TOGGLE   = OCT_KEY_TAB;
const Oct_Key BUTTON_NEXT_TAB      = OCT_KEY_C;
const Oct_Key BUTTON_PREVIOUS_TAB  = OCT_KEY_X;
const Oct_Key BUTTON_LEFT          = OCT_KEY_LEFT;
const Oct_Key BUTTON_RIGHT         = OCT_KEY_RIGHT;
const Oct_Key BUTTON_UP            = OCT_KEY_UP;
const Oct_Key BUTTON_DOWN          = OCT_KEY_DOWN;

const Traits BLUR_SELF_DAMAGE_TRAITS = {
        .Attack.blur = true,
        .Attack.blood = true,
        .occult = true,
};
const Traits RUST_SELF_DAMAGE_TRAITS = {
        .Attack.melee = true,
        .Attack.blood = true,
};
