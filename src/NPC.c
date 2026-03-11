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
    slot->info.drawn_type = DRAWN_TYPE_TEXTURE;
    slot->info.texture = oct_GetAsset(g_game.assets, "characters/slime.png");
    slot->aggro_range = 4;
    get_starting_weapon(WEAPON_TYPE_OTHER, &slot->starting_weapon);
}
