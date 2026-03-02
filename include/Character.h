/// \brief Character specific tools
#pragma once
#include "Structs.h"

// For character creation and most enemies
void get_starting_weapon(WeaponType weapon_type, Weapon *out);

// For soul-bound weapons
void get_weapon(WeaponType weapon_type, Rarity rarity, Weapon *out);

// Returns a random statblock, obeying stat allocation rules
void random_statblock(Statblock *out);

// Prints a statblock to the console
void print_statblock(Statblock *s);

// Returns a pointer to the skill pip in the stat block for the given base stat and skill index
int32_t *get_skill_pip(Statblock *s, int32_t base_stat_index, int32_t skill_index);

// Returns skill name given the base stat and skill index
const char *get_skill_name(int32_t base_stat_index, int32_t skill_index);

// Does boilerplate to create a character in place
void character_create(Statblock *starting_stats, Character *out);

// Draws a character wherever specified
void character_draw(Character *c, Oct_Vec2 position);

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
bool character_is_alive(Character *c);

// Returns actual damage taken
int32_t character_take_damage(Character *c, int32_t damage, Traits *source_traits);

// Returns actual evade pips
int32_t character_evade_pips(Character *c);

// Moves a character (including the visual portion and not just the logic). Returns false if the character can't move
bool character_move(Character *c, const Position new_position);