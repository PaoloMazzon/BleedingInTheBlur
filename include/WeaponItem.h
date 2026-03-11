/// \brief Specifically weapon and item effects and creation
#pragma once
#include "Structs.h"

// For character creation and most enemies
void get_starting_weapon(WeaponType weapon_type, Weapon *out);

// For soul-bound weapons
void get_weapon(WeaponType weapon_type, Rarity rarity, Weapon *out);