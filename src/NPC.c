#include "Util.h"
#include "Game.h"
#include "Character.h"
#include "WeaponItem.h"

void create_slime(Character *slot, Position pos) {
    Statblock sb = {
            .grit = 6,
            .suffer = 1,
            .healing = 4,
            .deaths_door = 1,
            .attrition = 1,

            .learning = 2,
            .occult = 0,
            .herbalism = 0,
            .tactics = 0,
            .cartography = 0,

            .wits = 2,
            .perception = 1,
            .escape = 4,
            .deception = 0,
            .trapping = 0,

            .martial = 4,
            .blades = 0,
            .marksman = 0,
            .grappler = 3,
            .evade = 1,
    };
    Traits traits = {
            .Character.lazy = true,
            .Character.hostile = true,
    };
    character_create(&sb, pos, slot);
    slot->info.name = "Slime";
    slot->info.traits = traits;
    info_set_sprite(&slot->info, oct_GetAsset(g_game.assets, "characters/slime.json"));
    slot->aggro_range = 4;
    get_starting_weapon(WEAPON_TYPE_OTHER, &slot->starting_weapon);
}

void create_zombie(Character *slot, Position pos) {
    Statblock sb = {
            .grit = 6,
            .suffer = 4,
            .healing = 0,
            .deaths_door = 0,
            .attrition = 4,

            .learning = 2,
            .occult = 0,
            .herbalism = 0,
            .tactics = 0,
            .cartography = 0,

            .wits = 2,
            .perception = 1,
            .escape = 1,
            .deception = 0,
            .trapping = 0,

            .martial = 5,
            .blades = 0,
            .marksman = 0,
            .grappler = 3,
            .evade = 0,
    };
    Traits traits = {
            .Character.lazy = true,
            .Character.hostile = true,
            .Character.blood_thirsty = true,
            .Character.wet = true,
            .Character.dumb = true,
            .occult = true,
    };
    character_create(&sb, pos, slot);
    slot->info.name = "Zombie";
    slot->info.traits = traits;
    slot->aggro_range = 6;
    info_set_sprite(&slot->info, oct_GetAsset(g_game.assets, "characters/zombie.json"));

    // Weapon
    get_starting_weapon(WEAPON_TYPE_OTHER, &slot->starting_weapon);
    slot->starting_weapon.damage = 3;
    slot->starting_weapon.info.traits.Attack.withering = true;
}

void create_skeleton(Character *slot, Position pos) {
    Statblock sb = {
            .grit = 4,
            .suffer = 4,
            .healing = 0,
            .deaths_door = 0,
            .attrition = 4,

            .learning = 2,
            .occult = 0,
            .herbalism = 0,
            .tactics = 0,
            .cartography = 0,

            .wits = 4,
            .perception = 1,
            .escape = 1,
            .deception = 0,
            .trapping = 0,

            .martial = 5,
            .blades = 3,
            .marksman = 0,
            .grappler = 3,
            .evade = 2,
    };
    Traits traits = {
            .Character.lazy = true,
            .Character.hostile = true,
            .Character.blood_thirsty = true,
            .Character.wet = true,
            .Character.dumb = true,
            .occult = true,
    };
    character_create(&sb, pos, slot);
    slot->info.name = "Skeleton";
    slot->info.traits = traits;
    slot->aggro_range = 6;
    info_set_sprite(&slot->info, oct_GetAsset(g_game.assets, "characters/skeleton.json"));

    // Weapon
    get_starting_weapon(WEAPON_TYPE_SWORD, &slot->starting_weapon);
    slot->starting_weapon.damage = 4;
    slot->starting_weapon.info.traits.Attack.rusted = true;
}
