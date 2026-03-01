/// \brief Character specific tools
#pragma once
#include "Structs.h"

void character_create(Statblock *starting_stats, Character *out);
void character_get_current_stats(Character *c, Statblock *out);
int32_t character_max_hp(Character *c);
int32_t character_max_mana(Character *c);
int32_t character_movement(Character *c);
int32_t character_crit_chance(Character *c);