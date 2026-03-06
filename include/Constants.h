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

// Controls
extern const Oct_Key BUTTON_ATTACK_VIEW;
extern const Oct_Key BUTTON_STAT_TOGGLE;
extern const Oct_Key BUTTON_CONFIRM;
extern const Oct_Key BUTTON_LEFT;
extern const Oct_Key BUTTON_RIGHT;
extern const Oct_Key BUTTON_UP;
extern const Oct_Key BUTTON_DOWN;

// Glyphs in the dice font
#define GLYPH_D8 "."
#define GLYPH_D6 "/"
#define GLYPH_OUT_OF ":"
#define GLYPH_ARROW "="
#define GLYPH_UP ";"
#define GLYPH_DOWN "<"
#define GLYPH_POINT_RIGHT "="

#define MAX_ALARMS 100
#define INVENTORY_SIZE 5
#define MAX_CHARACTERS 100
#define MAX_ITEMS 100
#define MAX_LABELS 100
#define MAX_USER_INPUT_SIZE 32
#define MAX_POPUP_STACK 10

// These are compiler preprocessors for easy access
#define CAMERA_ID 1
#define LABELS_ID_START 100 // needs at least MAX_LABELS
#define ATTACK_CURSOR_ID_START 200 // needs at least 9