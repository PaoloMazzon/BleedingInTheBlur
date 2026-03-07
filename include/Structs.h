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
typedef int32_t IntRange[2];

// These are a combination of an index into the popup stack and the generation of the popup
typedef uint64_t PopupDicePointer;
typedef uint64_t PopupInputPointer;

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
    LEVEL_INDEX_MENU        = 0,
    LEVEL_INDEX_FLOOR_1     = 1,
    LEVEL_INDEX_FLOOR_2     = 2,
    LEVEL_INDEX_FLOOR_3     = 3,
    LEVEL_INDEX_FLOOR_4     = 4,
    LEVEL_INDEX_FLOOR_5     = 5,
    LEVEL_INDEX_FLOOR_FINAL = 6,
    LEVEL_INDEX_QUIT        = 7, // quit the game
} LevelIndex;
typedef enum {
    DRAWN_TYPE_SPRITE  = 0,
    DRAWN_TYPE_TEXTURE = 1,
} DrawnType;
typedef enum {
    TILE_CONTENTS_TYPE_NONE      = 0,
    TILE_CONTENTS_TYPE_CHARACTER = 1,
    TILE_CONTENTS_TYPE_ITEM      = 2,
    TILE_CONTENTS_TYPE_WALL      = 3,
} TileContentsType;
typedef enum {
    WEAPON_TYPE_SWORD    = 0,
    WEAPON_TYPE_SPEAR    = 1,
    WEAPON_TYPE_BOW      = 2,
    WEAPON_TYPE_CROSSBOW = 3,
    WEAPON_TYPE_DAGGER   = 4,
    WEAPON_TYPE_OTHER    = 5, // Things like monster claws
    WEAPON_TYPE_MAX      = 6,
} WeaponType;
typedef enum {
    RARITY_COMMON   = 0,
    RARITY_UNCOMMON = 1,
    RARITY_RARE     = 2,
    RARITY_MYTHIC   = 3,
    RARITY_MAX      = 4,
} Rarity;
typedef enum {
    ATTACK_FAVOUR_NEUTRAL = 0, // no change to pips
    ATTACK_FAVOUR_GOOD    = 1, // the circumstances result in extra pips
    ATTACK_FAVOUR_BAD     = 2, // the circumstances result in fewer pips
} AttackFavour;

// These should all be bools
typedef struct Traits_s {
    struct {
        bool melee; // melee attack
        bool ranged; // ranged attack
        bool blade; // bladed weapon
        bool improvised; // improvised attack
        bool heavy; // heavy weapon
    } Attack; // traits for attacks/weapons

    struct {
        bool human;
        bool undying; // doesn't die
        bool hostile; // attacks the player
        bool lazy; // will only pursue the player if the player is close
        bool nimble; // good against ranged
    } Character; // traits for characters

    // Traits that can be either
    bool occult;
    bool holy;
} Traits;

// These are countdowns, above 0 means its active this turn
typedef struct StatusEffects_s {
    int32_t grappled; // can't move, attackers get +1D
    int32_t poisoned; // damage each turn
} StatusEffects;

// Information about any in-game object, like sprite and name and traits
typedef struct ObjectInfo_s {
    // Every object has a name
    const char *name;

    // For Octarine interpolation
    uint64_t id;

    // Objects are either sprites or textures
    DrawnType drawn_type;
    union {
        struct {
            Oct_Sprite sprite;
            Oct_SpriteInstance sprite_instance;
        };
        Oct_Texture texture;
    };

    // To easily lerp characters/items to where they should be
    Oct_Vec2 target_position;
    Oct_Vec2 actual_position;
    float rotation;
    float scale_x;
    float facing_direction;

    // These matter for skills and items and other things
    Traits traits;
} ObjectInfo;

// Something like a wall or door or trap
typedef struct Tile_s {
    int32_t index; // index in the tileset
    int32_t max_hp;
    int32_t current_hp; // some tiles can be destroyed
} Tile;

typedef struct Item_s {
    ObjectInfo info;
    ItemEnterInventoryCallback enter_inventory_callback;
    ItemExitInventoryCallback exit_inventory_callback;
    ItemUseCallback use_callback;
} Item;

// For keeping track of time (in terms of frames)
typedef struct Timer_s {
    int32_t start_frame;
    int32_t end_frame;
} Timer;

typedef struct Alarm_s {
    int32_t turns_left;
    AlarmCallback callback;
} Alarm;

typedef struct Weapon_s {
    ObjectInfo info;
    WeaponType type;
    Statblock bonus_stats;
    int32_t damage;
    int32_t range;
} Weapon;

typedef struct LevelGenerationParameters_s {
    IntRange level_size;
    IntRange room_min_size;
    IntRange room_max_size;
    IntRange room_count;
    IntRange extra_hallways;
} LevelGenerationParameters;

// A character in the game
struct Character_s {
    ObjectInfo info;
    Position pos;
    Statblock initial_statblock; // this should never change
    Statblock bonus_statblock; // additive to base
    Alarm alarms[MAX_ALARMS];
    Item items[INVENTORY_SIZE];
    StatusEffects status_effects; // status effects are decremented every turn

    // Weapons
    union {
        struct {
            Weapon starting_weapon;
            Weapon soul_bound_weapon;
        };
        Weapon weapons[2];
    };
    int32_t active_weapon;
    bool has_soul_bound_weapon;

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

    // Timer to control when the character can start facing another way after attacking
    Timer face_away_timer;
};

// Things that can be on a tile in the dungeon
typedef struct TileContents_s {
    TileContentsType type;
    union {
        Character *character;
        Item *item;
        Tile tile; // tiles are owned by the TileContents they reside in
    };
} TileContents;

typedef struct Label_s {
    const char *label;
    Oct_Vec2 position;
    bool needs_to_be_freed;
    int32_t max_ticks;
    int32_t ticks_remaining;
    Oct_Colour colour;
    bool dice_font;
} Label;

typedef enum {
    POPUP_TYPE_DICE = 0,
    POPUP_TYPE_MESSAGE = 1,
    POPUP_TYPE_TEXT_INPUT = 2,
} PopupType;

// Pop-up like an input dialogue or dice rolling
typedef struct {
    PopupType type;

    // Generation is incremented when the popup is closed. value_available is set to true
    // when the value is made ready (dice finish rolling for example), and it is set to
    // false when the user grabs a value out of it. This means that the value grabbing function
    // can be absolutely sure the value coming out of the popup belongs to it, and popups
    // are only reused once value_available is false again.
    uint32_t generation;
    bool value_available;

    union {
        struct {
            int32_t pips;
            int32_t dc;
            int32_t result;
        } Dice;
        struct {
            const char *message;
            bool needs_to_be_freed;
        } Message;
        struct {
            const char *message;
            bool needs_to_be_freed;
            char user_input[MAX_USER_INPUT_SIZE];
        } TextInput;
    };
} Popup;

// Information for the level to track the player's attack view
typedef struct AttackView_s {
    Position attack_cursor;
    Oct_Vec2 cursor_real_pos; // for tweening
} AttackView;

typedef enum {
    // Player may move, pause, trigger the attack view, attack an adjacent tile, or pass
    LEVEL_STATE_PLAYER_INTERACTION = 0,

    // Player is browsing their inventory
    LEVEL_STATE_PLAYER_INVENTORY = 1,

    // Player is browsing their stats
    LEVEL_STATE_PLAYER_STATS = 2,

    // Player may select a spell then move to the attack view
    LEVEL_STATE_PLAYER_SPELLS = 3,

    // Player is in attack view and may choose a tile within range to attack with a weapon or spell
    LEVEL_STATE_PLAYER_ATTACK = 4,

    // Enemies will take their turns sequentially, also has a brief pause at the start to allow player's turn to unwind
    LEVEL_STATE_ENEMY_TURN = 5,

    // Player is dead and may return to menu
    LEVEL_STATE_PLAYER_DEATH = 6,
} LevelState;

// Levels are a state machine where the state is stored in state
// HOWEVER. If there is something in the popup stack then that takes priority
// until the popup stack is empty
typedef struct Level_s {
    // Array of tile contents representing the whole level grid, size is width * height
    TileContents *tiles;
    int32_t level_width;
    int32_t level_height;

    // Current state of the level, user interaction and phases are FSM
    LevelState state;
    Popup popup_stack[MAX_POPUP_STACK]; // there can be multiple in a row
    int32_t popup_stack_pointer;
    AttackView attack_view;

    // For drawing
    Oct_Tilemap tilemap;
    bool stats_toggle; // toggle for drawing player stats
    Oct_Texture level_tex;

    Character characters[MAX_CHARACTERS];
    Item items[MAX_ITEMS];
    Label labels[MAX_LABELS];

    // If the player does something, and it isn't an extra turn, the world gets a turn
    bool world_turn;

    // For attack animations
    struct {
        int32_t damage;
        bool successful;
        Timer animation_timer;
        Oct_Texture tex;
        bool ranged; // if the tex should return to the sender
        Character *attacker;
        Character *receiver;
        Traits traits; // traits associated with the attack
        char buffer[MAX_BUFFER_LENGTH + 1];
    } Attack;
} Level;

typedef struct Game_s {
    Oct_AssetBundle assets;
    Oct_Allocator allocator;
    Oct_Texture backbuffer; // for drawing upscaled
    Oct_Camera world_camera; // in-game-world camera
    Oct_Camera ui_camera; // ui camera (it'll be virtual screen size)
    Oct_Camera render_camera; // for the final backbuffer

    // Level in the dungeon
    Level current_level;
    LevelIndex level_index;

    // Where the camera is looking
    Position camera_center;
    float zoom; // usually 1

    // Single source of truth for the player, everything else is a reference to this
    Character player;

    int32_t frame; // increments 1x per frame
} Game;