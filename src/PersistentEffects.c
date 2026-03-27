#include <assert.h>
#include "PersistentEffects.h"
#include "Game.h"

// Places the given parameters into an available alarm slot
static void apply_alarm(Character *c, AlarmCallback callback, Oct_Texture icon, int32_t turns) {
    int32_t alarm_slot = -1;
    for (int32_t i = 0; i < MAX_ALARMS && alarm_slot == -1; i++) {
        if (!alarm_is_active(c, i)) alarm_slot = i;
    }
    assert(alarm_slot != -1);
    c->alarms[alarm_slot].turns_left = turns;
    c->alarms[alarm_slot].callback = callback;
    c->alarms[alarm_slot].icon = icon;
}

static bool undo_crushed_hand_status_effect(Character *c) {
    c->bonus_statblock.marksman += 1;
    return true;
}

void apply_crushed_hand_status_effect(Character *c) {
    c->bonus_statblock.marksman -= 1;
    apply_alarm(c, undo_crushed_hand_status_effect, oct_GetAsset(g_game.assets, "icons/crushedhand.png"), 2);
}

bool alarm_is_active(Character *c, int32_t index) {
    assert(index >= 0 && index < MAX_ALARMS);
    return c->alarms[index].turns_left > 0;
}
