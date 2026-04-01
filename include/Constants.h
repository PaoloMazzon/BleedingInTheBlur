/// \brief All game constants
#pragma once

extern const char *BASE_STAT_NAMES[4];
extern const char *GRIT_STAT_NAMES[4];
extern const char *LEARNING_STAT_NAMES[4];
extern const char *WITS_STAT_NAMES[4];
extern const char *MARTIAL_STAT_NAMES[4];
extern const int32_t SOUL_BOUND_WEAPON_SLOT;
extern const int32_t STARTING_WEAPON_SLOT;
extern const char *WEAPON_TYPE_NAMES[6];
extern const char *RARITY_NAMES[4];
extern const char *GAME_STATES[7];
extern const char *SPRITE_LAYER_NAMES[6];

// How many base stat points you have, no stat can have more than BASE_STAT_MAX
extern const int32_t STARTING_BASE_STAT_POINTS;

// Maximum value of a base stat
extern const int32_t BASE_STAT_MAX;

// How many skill pips players get at the start of the game
extern const int32_t STARTING_SKILL_PIPS;

// Constant that defines a timestamp as being not in use
extern const int32_t TIMESTAMP_NOT_IN_USE;

// Animation durations
extern const int32_t ATTACK_ANIMATION_DURATION;

// Cell width/height for the rendered game
extern const float CELL_WIDTH;
extern const float CELL_HEIGHT;
extern const float VIRTUAL_WIDTH;
extern const float VIRTUAL_HEIGHT;
extern const float GAME_VIEW_WIDTH;
extern const float GAME_VIEW_HEIGHT;

// Tiles
extern const int32_t TILE_DOOR_OPEN;
extern const int32_t TILE_DOOR_CLOSED;
extern const int32_t TILE_FLOOR_SHADOW_LEFT; // These three are for the shadows that cast on the floor from the right
extern const int32_t TILE_FLOOR_SHADOW_BELOW;
extern const int32_t TILE_FLOOR_SHADOW_CORNER;
extern const int32_t TILE_WALL_SHADOW_LEFT; // directions represent where the empty space is
extern const int32_t TILE_WALL_SHADOW_RIGHT;
extern const int32_t TILE_WALL_SHADOW_ABOVE;
extern const int32_t TILE_WALL_SHADOW_BELOW;
extern const int32_t TILE_WALL_SHADOW_OUTER_CORNER_TR;
extern const int32_t TILE_WALL_SHADOW_OUTER_CORNER_TL;
extern const int32_t TILE_WALL_SHADOW_OUTER_CORNER_BR;
extern const int32_t TILE_WALL_SHADOW_OUTER_CORNER_BL;
extern const int32_t TILE_WALL_SHADOW_INNER_CORNER_TR;
extern const int32_t TILE_WALL_SHADOW_INNER_CORNER_TL;
extern const int32_t TILE_WALL_SHADOW_INNER_CORNER_BR;
extern const int32_t TILE_WALL_SHADOW_INNER_CORNER_BL;
extern const int32_t TILE_WALL_SHADOW_ABOVE_BELOW;
extern const int32_t TILE_WALL_SHADOW_ABOVE_BELOW_RIGHT;
extern const int32_t TILE_WALL_SHADOW_ABOVE_BELOW_LEFT;
extern const int32_t TILE_WALL_SHADOW_LEFT_RIGHT;
extern const int32_t TILE_WALL_SHADOW_LEFT_RIGHT_TOP;
extern const int32_t TILE_WALL_SHADOW_LEFT_RIGHT_BOTTOM;
extern const int32_t TILE_STAIRS_UP;
extern const int32_t TILE_STAIRS_DOWN;

// Controls
extern const Oct_Key BUTTON_ATTACK_VIEW;
extern const Oct_Key BUTTON_ITEM_SWAP;
extern const Oct_Key BUTTON_ITEM_USE;
extern const Oct_Key BUTTON_MENU_TOGGLE;
extern const Oct_Key BUTTON_NEXT_TAB;
extern const Oct_Key BUTTON_PREVIOUS_TAB;
extern const Oct_Key BUTTON_CONFIRM;
extern const Oct_Key BUTTON_SWAP_WEAPON;
extern const Oct_Key BUTTON_LEFT;
extern const Oct_Key BUTTON_RIGHT;
extern const Oct_Key BUTTON_UP;
extern const Oct_Key BUTTON_DOWN;

struct Traits_s;
extern const struct Traits_s BLUR_SELF_DAMAGE_TRAITS;
extern const struct Traits_s RUST_SELF_DAMAGE_TRAITS;

// Glyphs in the dice font
#define GLYPH_D8 "."
#define GLYPH_D6 "/"
#define GLYPH_OUT_OF ":"
#define GLYPH_ARROW "="
#define GLYPH_UP ";"
#define GLYPH_DOWN "<"
#define GLYPH_POINT_RIGHT "="

#define MAX_ALARMS 100
#define INVENTORY_SIZE 4
#define MAX_CHARACTERS 100
#define MAX_ITEMS 100
#define MAX_LABELS 100
#define MAX_PROPS 100
#define MAX_USER_INPUT_SIZE 32
#define MAX_POPUP_STACK 10
#define MAX_BUFFER_LENGTH 50
#define MAX_SPRITE_LAYERS 6
#define MAX_COLOURS 10

// These are compiler preprocessors for easy access
#define LABELS_ID_START 100 // needs at least MAX_LABELS
#define ATTACK_CURSOR_ID_START 200 // needs at least 9
#define ITEM_ID_STAR 300 // items on the hud
#define OBJECT_ID_START 1000000 // when objects start getting assigned permanent IDs
#define OBJECT_ID_RESERVED_MULTIPLIER 1000 // each object has the id [id * 1000, id * 1000 + 1000) reserved in addition to its original id
#define                         CAMERA_ID 1
#define               WEAPON_INDICATOR_ID 2
#define                   MOVEMENT_BAR_ID 3
#define                 ENEMY_HUD_NAME_ID 4
#define            ENEMY_HUD_HEALTHBAR_ID 5
#define ENEMY_HUD_HEALTHBAR_BACKGROUND_ID 6
#define                  ITEM_SELECTOR_ID 7
#define                ITEM_BACKGROUND_ID 8
#define                  POPUP_POINTER_ID 9
#define                      ITEM_NAME_ID 10
#define                     PROJECTILE_ID 11