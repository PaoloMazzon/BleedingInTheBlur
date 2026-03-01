/// \brief Character specific tools
#pragma once
#include "Structs.h"

// Does boilerplate to create a character in place
void character_create(Statblock *starting_stats, Character *out);

// Returns the current statblock for a character (initial stats + bonus)
void character_get_current_stats(Character *c, Statblock *out);

// Returns the actual maximum hp for a character
int32_t character_max_hp(Character *c);

// Returns the actual maximum mana for a character
int32_t character_max_mana(Character *c);

// Returns the actual movement score for a character
int32_t character_movement(Character *c);

// Returns the actual crit chance for a character
int32_t character_crit_chance(Character *c);

// Returns false if the character is dead
bool character_alive(Character *c);

// Returns actual damage taken
int32_t character_take_damage(Character *c, int32_t damage, Traits *source_traits);
