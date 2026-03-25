# Bleeding in the Blur
Roguelike game

HEAVILY WIP

![BleedingInTheBlur_iA8PQPm7Xh](https://github.com/user-attachments/assets/96379b09-ed0a-4ecc-8bdd-2fbd184a90fe)

## Tech
I wrote this game in C because I really like the C language. Is it a good idea
for you to also write roguelike RGPs in C? No, go use Rust or something. I try
to break up the game logic across a lot of functions with really verbose names;
most functions should be pretty obvious and there are a lot of comments as well.
From a top-down perspective, the singleton `g_game` is an instance of the `Game`
struct and that struct stores the player (which is a `Character`) and the level
of type `Level`. The level is several grids, and each grid tile can store a
single character and a single item or weapon. 

The game is turn based, so barring tweening/animation logic, the player takes
their turn then each other character in the level 
(`g_game.current_level.characters`) takes their turn. Every 
item/weapon/character in the game has a `Traits` struct and that struct is
basically that object's fingerprint. NPCs behaviour is completely determined by
their traits, meaning there is no enemy-specific logic and instead a universal
"take turn" function for all NPCs (`character_take_turn@CharacterTurn.h`).

Characters have initial stats (in a `Statblock` struct) and those stats are
modified by their bonus statblock (which is in turn modified by items or status
effects) and their weapon. You would use `character_get_current_stats` to get
a particular character's current stats. Additionally, all skills current scores
are referred to as that skill's "pips." For instance, a character might have a
base "marksman" skill of 3 pips, but that character is carrying a hunter's charm
giving a +1 to marksman attacks and also has a spear equipped which gives an extra
+1 again. So their actual marksman skill is 5 pips. Each pip represents a single
6-sided die and each skill check rolls a 6-sided die for each skill pip you have
and a base 8-sided die as well.

All skill checks are either "rolling ups" or a "pip check." There are 4 base
stats: grit, learning, wits, and martial, and 4 derivative stats under each of
those. Base skills determine 1 thing each and also the skill DC (20 - the base
stat) for that category. For instance, with a grit of 5, your grit DC would be
20-5=15 meaning you would need to roll at least a 15 on grit-based skill checks
to succeed them (in the case of "pip checks"). Rolling "ups" means you must roll
x amount of the highest possible; ie on a D6 you must achieve a 6 and on a D8
you must achieve an 8. For example, evading an attack requires a 2 up evade 
roll. If you had 4 pips in evade, you would roll 1D8 and 4D6. If you rolled
`[8, 4, 3, 6, 1]`, you would have achieved the necessary 2 pips given that 8 and
6 were the highest possible values on those two dice. All dice rolling logic can
be found in `roll_ups@Util.c` and `roll_dice@Util.c`. Additionally, on damage
checks (which are all pip checks), any excess roll above the DC counts as bonus
damage. So if your DC for your attack is 14, and you roll a 17 on the attack, 
the total damage would be (weapon base damage + (17 - 14)). This is to provide
a reason to get as many pips as possible in your primary damaging skill.

Here are the base skills and their derivative skills:

 * Grit - Suffer, healing, death's door (1 up), attrition
 * Learning - Occult, herbalism, tactics (1 up), cartography
 * Wits - Perception, escape (2 ups), deception, trapping
 * Martial - Blades, marksman, grappler, evade (2 ups)

Any skill that says the number of ups will always be rolled as an ups skill
whereas all others are always pip checks.

## Important Places

 - Most trait-related effects go in `process_attack_traits@AttackAnimation.c`
   (Evade, escape, critical hits, and death's door are exceptions)
 - All dice-related bonuses go in `character_get_attack_stats@Character.c`
 - All character-related behavioural code goes in 
   `character_take_turn@CharacterTurn.c`
 - Weapons and items creation code is all in `WeaponItem.h`, all code that 
   defines item-related callbacks is in `ItemCallbacks.c`, use 
   `SpellAttackProfile@ItemCallbacks.c` to store spell attack details
 - Any alarm-related code (alarms are status effects) are in 
   `PersistentEffeccts.h`

## License
The code in this repository is all MIT licensed. Feel free to use it to your
heart's content. Don't use the stuff in `data/`.

### Fonts

| Name    | Creator         | Link |
|---------|-----------------|------|
| Alagard | Hewett Tsoi     | [https://www.dafont.com/alagard.font](https://www.dafont.com/alagard.font) |
| Early GameBoy | Jimmy Campbell  | https://www.dafont.com/early-gameboy.font |

### Libraries
This project is written "from-scratch," meaning it's written in a low-level lang
but by no means would this project have been possible without these amazing
libraries (especially the first two, no bias involved there).

 * [Octarine](https://github.com/PaoloMazzon/Octarine) - MIT
 * [Vulkan2D](https://github.com/PaoloMazzon/Vulkan2D) - zlib
 * [SDL3](https://wiki.libsdl.org/SDL3/FrontPage) - zlib
 * [SDL3_ttf](https://wiki.libsdl.org/SDL3_ttf/FrontPage) - zlib
 * [Slang](https://shader-slang.org/) - Apache 2.0
 * [cJSON](https://github.com/DaveGamble/cJSON) - MIT
 * [mimalloc](https://github.com/microsoft/mimalloc) - MIT
 * [PhysicsFS](https://github.com/icculus/physfs) - zlib
 * [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - MIT
 * [stb_vorbis](https://github.com/nothings/stb/tree/master) - MIT
 * [stb_image](https://github.com/nothings/stb/tree/master) - MIT
 * [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) - MIT