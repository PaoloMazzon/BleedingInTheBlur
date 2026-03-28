#include <assert.h>
#include "PersistentEffects.h"
#include "Game.h"

// Use this to define an undo status effect function
#define UNDO_STATUS_EFFECT_DEFINE(name) static bool undo_##name##_status_effect(Character *c)

// Use this to get the function previously defined
#define UNDO_STATUS_EFFECT(name) undo_##name##_status_effect

// Use this to define the alarm each turn function
#define ALARM_EACH_TURN_DEFINE(name) bool alarm_turn_##name##_each_turn(Character *c, int32_t turns_remaining)

// Use this to get the alarm each turn function
#define ALARM_EACH_TURN(name) bool alarm_turn_##name##_each_turn

// Places the given parameters into an available alarm slot
static void apply_alarm(Character *c, AlarmCallback callback, AlarmTurnCallback turn_callback, Oct_Texture icon, int32_t turns) {
    int32_t alarm_slot = -1;
    for (int32_t i = 0; i < MAX_ALARMS && alarm_slot == -1; i++) {
        if (!alarm_is_active(c, i)) alarm_slot = i;
    }
    assert(alarm_slot != -1);
    c->alarms[alarm_slot].turns_left = turns;
    c->alarms[alarm_slot].callback = callback;
    c->alarms[alarm_slot].turn_callback = turn_callback;
    c->alarms[alarm_slot].icon = icon;
}

//////////////////////////////////////////////////////////////////////
const int32_t CRUSHED_HAND_REDUCED_MARKSMAN = 1;
UNDO_STATUS_EFFECT_DEFINE(crushed_hand) {
    c->bonus_statblock.marksman += CRUSHED_HAND_REDUCED_MARKSMAN;
    return true;
}

void apply_crushed_hand_status_effect(Character *c) {
    c->bonus_statblock.marksman -= CRUSHED_HAND_REDUCED_MARKSMAN;
    apply_alarm(c,
                UNDO_STATUS_EFFECT(crushed_hand),
                nullptr,
                oct_GetAsset(g_game.assets, "icons/crushedhand.png"),
                3);
}
//////////////////////////////////////////////////////////////////////
const int32_t WITHERED_REDUCED_MAX_HP = 2;
UNDO_STATUS_EFFECT_DEFINE(withered) {
    c->bonus_max_hp += WITHERED_REDUCED_MAX_HP;
    return true;
}

void apply_withered_status_effect(Character *c) {
    c->bonus_max_hp -= WITHERED_REDUCED_MAX_HP;
    apply_alarm(c,
                UNDO_STATUS_EFFECT(withered),
                nullptr,
                oct_GetAsset(g_game.assets, "icons/crushedhand.png"),
                10);
}
//////////////////////////////////////////////////////////////////////

bool alarm_is_active(Character *c, int32_t index) {
    assert(index >= 0 && index < MAX_ALARMS);
    return c->alarms[index].turns_left > 0;
}
