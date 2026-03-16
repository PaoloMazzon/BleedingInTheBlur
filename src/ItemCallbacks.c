#include <stdio.h>
#include "Character.h"

// In case I want to change the parameters of these callbacks later
#define ITEM_USE_CALLBACK(name) bool name(Character *c)
#define ENTER_INVENTORY_CALLBACK(name) bool name(Character *c)
#define EXIT_INVENTORY_CALLBACK(name) bool name(Character *c)

// rolls healing (against grit) and on a pass there is bonus hp
static int32_t get_bonus_hp(Character *c) {
    Statblock sb;
    character_get_current_stats(c, &sb);
    int32_t result = 0;
    int32_t dice_result = 0;
    if (roll_dice(sb.healing, statblock_get_dc(sb.grit), &dice_result))
        result = dice_result - statblock_get_dc(sb.grit) + 1;
    return result;
}

// dynamically allocates a string for a potion label
static const char *get_hp_string(int32_t base_healing, int32_t bonus_healing) {
    char *buf = oct_Malloc(g_game.allocator, 20);
    if (bonus_healing == 0)
        snprintf(buf, 19, "+%i hp", base_healing);
    else
        snprintf(buf, 19, "+%i HP!!", base_healing + bonus_healing);
    return buf;
}

ITEM_USE_CALLBACK(small_potion_callback) {
    int32_t base_hp = 5;
    const int32_t bonus_hp = get_bonus_hp(c);
    c->current_hp += base_hp + bonus_hp;
    create_label(get_hp_string(base_hp, bonus_hp), c->pos, (Oct_Colour){.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f, }, true);
    return true;
}
