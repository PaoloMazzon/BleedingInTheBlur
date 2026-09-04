#include <string.h>
#include <slog.h>
#include "WeaponItem.h"
#include "Game.h"
#include "Util.h"

// is this bad practice? maybe idk
#include "ItemCallbacks.c"

// What kinda item is this for the purposes of identification, doesn't need to be precise
typedef enum {
    IDENTIFICATION_BITMASK_SPELL  = 1 << 0,
    IDENTIFICATION_BITMASK_OCCULT = 1 << 1,
    IDENTIFICATION_BITMASK_POTION = 1 << 2,
} IdentificationBitmask;

void get_starting_weapon(WeaponType weapon_type, Weapon *out) {
    memset(out, 0, sizeof(Weapon));
    get_weapon(weapon_type, RARITY_COMMON, out);
}

void get_weapon(WeaponType weapon_type, Rarity rarity, Weapon *out) {
    memset(out, 0, sizeof(Weapon));
    // TODO: Do something with rarity
    if (weapon_type == WEAPON_TYPE_SWORD) {
        out->type = WEAPON_TYPE_SWORD;
        out->icon = oct_GetAsset(g_game.assets, "attacks/sword_icon.png");
        info_set_texture(&out->info, oct_GetAsset(g_game.assets, "attacks/sword.png"));
        out->info.name = "Sword";
        out->damage = 5;
        out->range = 1;
        out->bonus_stats.attrition = 1;
        out->info.traits.Attack.melee = true;
        out->info.traits.Attack.blade = true;
    } else if (weapon_type == WEAPON_TYPE_SPEAR) {
        out->type = WEAPON_TYPE_SPEAR;
        out->icon = oct_GetAsset(g_game.assets, "attacks/spear_icon.png");
        info_set_texture(&out->info, oct_GetAsset(g_game.assets, "attacks/spear.png"));
        out->info.name = "Spear";
        out->damage = 4;
        out->range = 2;
        out->bonus_stats.marksman = 1;
        out->info.traits.Attack.melee = true;
        out->info.traits.Attack.heavy = true;
        out->info.traits.Attack.blade = true;
    } else if (weapon_type == WEAPON_TYPE_BOW) {
        out->type = WEAPON_TYPE_BOW;
        out->icon = oct_GetAsset(g_game.assets, "attacks/bow_icon.png");
        out->info.name = "Bow";
        out->damage = 2;
        out->range = 4;
        out->info.traits.Attack.ranged = true;
    } else if (weapon_type == WEAPON_TYPE_CROSSBOW) {
        out->type = WEAPON_TYPE_CROSSBOW;
        out->icon = oct_GetAsset(g_game.assets, "attacks/crossbow_icon.png");
        out->info.name = "Crossbow";
        out->damage = 3;
        out->range = 4;
        out->bonus_stats.evade = -1;
        out->bonus_stats.marksman = 1;
        out->info.traits.Attack.ranged = true;
        out->info.traits.Attack.heavy = true;
    } else if (weapon_type == WEAPON_TYPE_DAGGER) {
        out->type = WEAPON_TYPE_DAGGER;
        out->icon = oct_GetAsset(g_game.assets, "attacks/dagger.png");
        out->info.name = "Dagger";
        out->damage = 2;
        out->range = 1;
        out->bonus_stats.evade = 1;
        out->info.traits.Attack.improvised = true;
        out->info.traits.Attack.exploit = true;
        out->info.traits.Attack.blade = true;
        out->info.traits.Attack.melee = true;
    } else if (weapon_type == WEAPON_TYPE_OTHER) {
        out->type = WEAPON_TYPE_OTHER;
        out->icon = oct_GetAsset(g_game.assets, "");
        out->info.name = "Claw";
        out->damage = 2;
        out->range = 1;
        out->info.traits.Attack.improvised = true;
        out->info.traits.Attack.melee = true;
    } else {
        slog_fatal("Weapon type %i hasn't been implemented.", weapon_type);
    }
}

bool use_item(Item *item, Character *c) {
    if (item->use_callback) {
        item->use_callback(c);
        item->charges_remaining -= 1;
        return item->charges_remaining > 0;
    }
    return true;
}

// Sets an item to reasonable defaults
static void prep_item(Item *out, const char *name, const char *extended_name, int32_t charges, Oct_Texture tex) {
    memset(out, 0, sizeof(Item));
    out->info.name = name;
    out->real_name = extended_name;
    out->charges_remaining = charges;
    out->charges = charges;
    info_set_texture(&out->info, tex);
    out->info.id = new_oct_id();
}

// Attempts to identify an item based on a few factors
static void attempt_identification(Item *item, IdentificationBitmask type) {
    Statblock sb;
    character_get_current_stats(&g_game.player, &sb);
    int32_t pips = 0;
    const int32_t dc = statblock_get_dc(sb.learning);
    if (type & IDENTIFICATION_BITMASK_OCCULT || type & IDENTIFICATION_BITMASK_SPELL)
        pips += sb.occult;
    if (type & IDENTIFICATION_BITMASK_POTION)
        pips += sb.herbalism;
    item->identified = roll_dice(pips, dc, nullptr);
}

void get_small_health_potion(Item *out) {
    prep_item(
            out,
            "Potion",
            "Small heal",
            1,
            oct_GetAsset(g_game.assets, "items/healthpotion.png"));

    out->type = ITEM_TYPE_POTION;
    out->use_callback = small_potion_use_callback;

    attempt_identification(out, IDENTIFICATION_BITMASK_POTION);
}

void get_evil_rock(Item *out) {
    prep_item(
            out,
            "Evil Rock",
            "Occult weapon",
            5,
            oct_GetAsset(g_game.assets, "items/rock.png"));

    out->type = ITEM_TYPE_ATTACK_SPELL;
    out->use_callback = evil_rock_use_callback;
    out->get_traits_callback = evil_rock_get_traits_callback;

    attempt_identification(out, IDENTIFICATION_BITMASK_SPELL | IDENTIFICATION_BITMASK_OCCULT);
}