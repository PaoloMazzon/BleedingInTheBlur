/// \brief All the structs and other related datatypes in the game
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <oct/Octarine.h>
#include "Constants.h"

#ifndef nullptr
#define nullptr NULL
#endif // nullptr

// Types
typedef struct Character_s Character;
typedef bool (*AlarmCallback)(Character *);
typedef bool (*ItemEnterInventoryCallback)(Character *);
typedef bool (*ItemExitInventoryCallback)(Character *);
typedef bool (*ItemUseCallback)(Character *);
typedef int32_t Position[2];

// Stats are a mess, just collapse them in your editor
typedef struct Statblock_s {
    // Base stats can be accessed via their name or base_stat[BASE_STAT_TYPE_*]
    union {
        struct {
            int32_t grit;
            int32_t learning;
            int32_t wits;
            int32_t martial;
        };
        int32_t base_stats[4];
    };

    // Same for derivative stats
    union {
        struct {
            int32_t suffer;
            int32_t healing;
            int32_t deaths_door;
            int32_t attrition;
        };
        int32_t grit_stats[4];
    };

    union {
        struct {
            int32_t occult;
            int32_t herbalism;
            int32_t tactics;
            int32_t cartography;
        };
        int32_t learning_stats[4];
    };

    union {
        struct {
            int32_t perception;
            int32_t escape;
            int32_t deception;
            int32_t trapping;
        };
        int32_t wits_stats[4];
    };

    union {
        struct {
            int32_t blades;
            int32_t marksman;
            int32_t grappler;
            int32_t evade;
        };
        int32_t martial_stats[4];
    };
} Statblock;
typedef enum {
    BASE_STAT_TYPE_GRIT     = 0,
    BASE_STAT_TYPE_LEARNING = 1,
    BASE_STAT_TYPE_WITS     = 2,
    BASE_STAT_TYPE_MARTIAL  = 3,
    BASE_STAT_TYPE_MAX      = 4
} BaseStatType;
typedef enum {
    GRIT_STAT_TYPE_SUFFER      = 0,
    GRIT_STAT_TYPE_HEALING     = 1,
    GRIT_STAT_TYPE_DEATHS_DOOR = 2,
    GRIT_STAT_TYPE_ATTRITION   = 3,
    GRIT_STAT_TYPE_MAX         = 4
} GritStatType;
typedef enum {
    LEARNING_STAT_TYPE_OCCULT      = 0,
    LEARNING_STAT_TYPE_HERBALISM   = 1,
    LEARNING_STAT_TYPE_TACTICS     = 2,
    LEARNING_STAT_TYPE_CARTOGRAPHY = 3,
    LEARNING_STAT_TYPE_MAX         = 4
} LearningStatType;
typedef enum {
    WITS_STAT_TYPE_PERCEPTION = 0,
    WITS_STAT_TYPE_ESCAPE     = 1,
    WITS_STAT_TYPE_DECEPTION  = 2,
    WITS_STAT_TYPE_TRAPPING   = 3,
    WITS_STAT_TYPE_MAX        = 4
} WitsStatType;
typedef enum {
    MARTIAL_STAT_TYPE_BLADES   = 0,
    MARTIAL_STAT_TYPE_MARKSMAN = 1,
    MARTIAL_STAT_TYPE_GRAPPLER = 2,
    MARTIAL_STAT_TYPE_EVADE    = 3,
    MARTIAL_STAT_TYPE_MAX      = 4
} MartialStatType;

typedef enum {
    DRAWN_TYPE_SPRITE = 0,
    DRAWN_TYPE_TEXTURE = 1,
} DrawnType;

// Information about any in-game object, like sprite and name and traits
typedef struct ObjectInfo_s {
    // Every object has a name
    const char *name;

    // Objects are either sprites or textures
    DrawnType drawn_type;
    union {
        struct {
            Oct_Sprite sprite;
            Oct_SpriteInstance sprite_instance;
        };
        Oct_Texture texture;
    };

    // These matter for skills and items and other things
    struct {
        bool human;
        bool occult;
    } Traits;
} ObjectInfo;

typedef struct Item_s {
    ObjectInfo info;
    ItemEnterInventoryCallback enter_inventory_callback;
    ItemExitInventoryCallback exit_inventory_callback;
    ItemUseCallback use_callback;
} Item;

typedef struct Alarm_s {
    int32_t turns_left;
    AlarmCallback callback;
} Alarm;

// A character in the game
struct Character_s {
    ObjectInfo info;
    Position pos;
    Statblock initial_statblock; // this should never change
    Statblock bonus_statblock; // additive to base
    Alarm alarms[MAX_ALARMS];

    // At 100 the character gets an extra turn
    int32_t cumulative_movement;

    // The base values are not recorded as they are derived from the current stat block
    int32_t bonus_movement; // raw 0-100
    int32_t bonus_crit_chance; // 1 = 1%, additive not multiplicative
    int32_t bonus_max_hp;
    int32_t bonus_max_mana;

    // Max hp and mana are derived from the current statblock
    int32_t current_hp;
    int32_t current_mana;
};