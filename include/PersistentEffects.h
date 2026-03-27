/// \brief Every persistent effect applies something to a given character then adds an alarm to that character to undo its effects at a later time
/// Items might apply these effects on use, but these are not meant to be item callbacks in and other themselves.
/// These are things that might get called on certain attacks or traps or whatever.
#pragma once
#include "Structs.h"

// Returns true if a given alarm is currently in use
bool alarm_is_active(Character *c, int32_t index);

// Temporary debuff to marksman from heavy weapons
void apply_crushed_hand_status_effect(Character *c);

// Reduce max hp by 2 for a number of turns
void apply_withered_status_effect(Character *c);
