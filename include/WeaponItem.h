/// \brief Specifically weapon and item effects and creation
#pragma once
#include "Structs.h"

// For character creation and most enemies
void get_starting_weapon(WeaponType weapon_type, Weapon *out);

// For soul-bound weapons
void get_weapon(WeaponType weapon_type, Rarity rarity, Weapon *out);

// Uses an item's use callback if available. Returns false if there are no charges remaining.
bool use_item(Item *item, Character *c);

// Creates a health potion that restores recovery hp
void get_small_health_potion(Item *out);