/// \brief All game constants
#pragma once

extern const char *BASE_STAT_NAMES[4];
extern const char *GRIT_STAT_NAMES[4];
extern const char *LEARNING_STAT_NAMES[4];
extern const char *WITS_STAT_NAMES[4];
extern const char *MARTIAL_STAT_NAMES[4];
extern const int32_t SOUL_BOUND_WEAPON_SLOT;
extern const int32_t STARTING_WEAPON_SLOT;

// How many base stat points you have, no stat can have more than BASE_STAT_MAX
extern const int32_t STARTING_BASE_STAT_POINTS;

// Maximum value of a base stat
extern const int32_t BASE_STAT_MAX;

// How many skill pips players get at the start of the game
extern const int32_t STARTING_SKILL_PIPS;

// Cell width/height for the rendered game
extern const float CELL_WIDTH;
extern const float CELL_HEIGHT;

#define MAX_ALARMS 100
#define INVENTORY_SIZE 5
#define MAX_CHARACTERS 100
#define MAX_ITEMS 100