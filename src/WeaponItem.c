#include <string.h>
#include "WeaponItem.h"
#include "Character.h"
#include "Game.h"

void get_starting_weapon(WeaponType weapon_type, Weapon *out) {
    memset(out, 0, sizeof(Weapon));
    get_weapon(weapon_type, RARITY_COMMON, out);
}

void get_weapon(WeaponType weapon_type, Rarity rarity, Weapon *out) {
    memset(out, 0, sizeof(Weapon));
    // TODO: Do something with rarity
    if (weapon_type == WEAPON_TYPE_SWORD) {
        out->type = WEAPON_TYPE_SWORD;
        out->info.texture = oct_GetAsset(g_game.assets, "attacks/sword.png");
        out->icon = oct_GetAsset(g_game.assets, "attacks/sword_icon.png");
        out->info.name = "Sword";
        out->damage = 5;
        out->range = 1;
        out->bonus_stats.attrition = 1;
        out->info.traits.Attack.melee = true;
        out->info.traits.Attack.blade = true;
    } else if (weapon_type == WEAPON_TYPE_SPEAR) {
        out->type = WEAPON_TYPE_SPEAR;
        out->info.texture = oct_GetAsset(g_game.assets, "attacks/spear.png");
        out->icon = oct_GetAsset(g_game.assets, "attacks/spear_icon.png");
        out->info.name = "Spear";
        out->damage = 4;
        out->range = 2;
        out->bonus_stats.marksman = 1;
        out->info.traits.Attack.melee = true;
        out->info.traits.Attack.heavy = true;
    } else if (weapon_type == WEAPON_TYPE_BOW) {
        out->type = WEAPON_TYPE_BOW;
        out->info.texture = oct_GetAsset(g_game.assets, "");
        out->icon = oct_GetAsset(g_game.assets, "");
        out->info.name = "Bow";
        out->damage = 2;
        out->range = 4;
        out->info.traits.Attack.ranged = true;
    } else if (weapon_type == WEAPON_TYPE_CROSSBOW) {
        out->type = WEAPON_TYPE_CROSSBOW;
        out->info.texture = oct_GetAsset(g_game.assets, "");
        out->icon = oct_GetAsset(g_game.assets, "");
        out->info.name = "Crossbow";
        out->damage = 3;
        out->range = 4;
        out->bonus_stats.evade = -1;
        out->bonus_stats.marksman = 1;
        out->info.traits.Attack.ranged = true;
        out->info.traits.Attack.heavy = true;
    } else if (weapon_type == WEAPON_TYPE_DAGGER) {
        out->type = WEAPON_TYPE_DAGGER;
        out->info.texture = oct_GetAsset(g_game.assets, "");
        out->icon = oct_GetAsset(g_game.assets, "");
        out->info.name = "Dagger";
        out->damage = 2;
        out->range = 1;
        out->bonus_stats.evade = 1;
        out->info.traits.Attack.improvised = true;
        out->info.traits.Attack.blade = true;
        out->info.traits.Attack.melee = true;
    } else if (weapon_type == WEAPON_TYPE_OTHER) {
        out->type = WEAPON_TYPE_OTHER;
        out->info.texture = oct_GetAsset(g_game.assets, "");
        out->icon = oct_GetAsset(g_game.assets, "");
        out->info.name = "Claw";
        out->damage = 2;
        out->range = 1;
        out->info.traits.Attack.improvised = true;
        out->info.traits.Attack.melee = true;
    } else {
        oct_Raise(OCT_STATUS_ERROR, true, "Weapon type %i hasn't been implemented.", weapon_type);
    }
}