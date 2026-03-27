#include <stdio.h>
#include <assert.h>
#include "Character.h"
#include "AttackAnimations.h"

// In case I want to change the parameters of these callbacks later
#define ITEM_USE_CALLBACK(name) bool name(Character *c)
#define ITEM_GET_TRAITS_CALLBACK(name) bool name(Character *c, Traits *t, int32_t *range)
#define ENTER_INVENTORY_CALLBACK(name) bool name(Character *c)
#define EXIT_INVENTORY_CALLBACK(name) bool name(Character *c)

typedef struct SpellAttackProfile_s {
    Traits traits;
    int32_t range;
} SpellAttackProfile;

static const SpellAttackProfile EVIL_ROCK_ATTACK_PROFILE = {
    .traits = {
            .Attack.ranged = true,
            .Attack.improvised = true,
            .occult = true,
    },
    .range = 5,
};

// dynamically allocates a string for a potion label
static const char *get_hp_string(int32_t base_healing, int32_t bonus_healing) {
    char *buf = oct_Malloc(g_game.allocator, 20);
    if (bonus_healing == 0)
        snprintf(buf, 19, "+%i hp", base_healing);
    else
        snprintf(buf, 19, "+%i HP!!", base_healing + bonus_healing);
    return buf;
}

// Gets a zero'd traits block valid until the next call to this
static Traits *get_traits_block() {
    static Traits t;
    memset(&t, 0, sizeof(Traits));
    return &t;
}

ITEM_USE_CALLBACK(small_potion_use_callback) {
    const int32_t base_hp = 5;
    const int32_t total_healed = character_heal(c, base_hp);
    create_label(get_hp_string(base_hp, total_healed - base_hp), c->pos, (Oct_Colour){.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f, }, true);
    return true;
}

ITEM_USE_CALLBACK(evil_rock_use_callback) {
    TileContents *t = level_get_tile(g_game.current_level.attack_view.attack_cursor);
    assert(t && t->type == TILE_CONTENTS_TYPE_CHARACTER);
    character_attempt_attack(c, &EVIL_ROCK_ATTACK_PROFILE.traits, t->character, 1);
    return true;
}

ITEM_GET_TRAITS_CALLBACK(evil_rock_get_traits_callback) {
    if (t) memcpy(t, &EVIL_ROCK_ATTACK_PROFILE.traits, sizeof(Traits));
    if (range) *range = EVIL_ROCK_ATTACK_PROFILE.range;
    return true;
}
