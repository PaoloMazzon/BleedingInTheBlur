#include <assert.h>
#include "PersistentEffects.h"

// Places the given parameters into an available alarm slot
static void apply_alarm(Character *c, AlarmCallback callback, Oct_Texture icon, int32_t turns) {
    int32_t alarm_slot = -1;
    for (int32_t i = 0; i < MAX_ALARMS && alarm_slot == -1; i++) {
        if (c->alarms[i].turns_left > 0) alarm_slot = i;
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
    apply_alarm(c, undo_crushed_hand_status_effect, OCT_NO_ASSET /* TODO: This */, 2);
}
