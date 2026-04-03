/// \brief Declares all game-related functions
#pragma once
#include "Structs.h"

// Global game state
extern Game g_game;

void menu_begin();
LevelIndex menu_update(); // return value is where to go
void menu_end();

void level_begin();
LevelIndex level_update(); // return value is where to go
void level_end();
void create_label(const char *text, const Position position, Oct_Colour colour, bool needs_to_be_freed);
void create_dice_label(const char *text, const Position position, Oct_Colour colour, bool needs_to_be_freed);
Character *level_get_character_slot();
// this can return null if it's out of bounds
TileContents *level_get_tile(Position pos);
TileContentsType level_get_tile_type(int32_t x, int32_t y); // returns TILE_CONTENTS_TYPE_NONE if out of bounds
bool level_in_attack_animation();
bool level_attack_animation_complete();
void level_get_spawn_point(Position out_tile);
// For player remembering out-of-sight parts of the level
int32_t level_get_tile_memory(Position pos); // get the visibility rating
void level_set_tile_memory(Position pos, int32_t visibility); // set the visibility rating
bool level_tile_is_remembered(Position pos); // just see if the player should remember it
bool level_tile_seen_this_turn(Position pos);
bool level_extra_player_turn();
void level_set_displayed_enemy(Character *c);
void level_transition_to_enemy_turns();
void level_next_enemy_turn();
bool tiles_have_walls_between(Position tile1, Position tile2);

// Move item/weapon from a tile into a new slot. It is an error to call this with a tile that does not contain an item/weapon
void level_extract_tile_item(Position tile, Item *out_item);
void level_extract_tile_weapon(Position tile, Weapon *out_weapon);

// Returns true if there is anything in the popup stack
bool popups_are_active();
void draw_and_update_popups();

// Popups are asynchronous "windows" whose output values need to be queried with their pointer
void popup_message(const char *text, bool needs_to_be_freed);
PopupInputPointer popup_input(const char *text, bool needs_to_be_freed);
PopupWeaponSelectPointer popup_weapon_select(Weapon *weapon);
PopupConfirmPointer popup_confirm(const char *text);
PopupItemSelectPointer popup_item_select(Item *item);

// Returns false if the popup is not yet resolved, true otherwise
bool popup_get_input(PopupInputPointer input_pointer, const char *out);
bool popup_get_weapon(PopupWeaponSelectPointer weapon_pointer, bool *selected);
bool popup_get_item(PopupItemSelectPointer item_pointer, int32_t *index); // -1 means no new item
bool popup_get_confirm(PopupConfirmPointer confirm_pointer, bool *selected);

void player_init(Position start_pos);
void player_update();
int32_t get_player_current_attack_range();

void *startup();
void *update(void *ptr);
void shutdown(void *ptr);
void set_draw_target(Oct_Texture tex);
void reset_draw_target(); // call after changing the draw target
void debug(const char *fmt, ...);

// Drawing items
void draw_item(Item *item, Oct_Vec2 position, float alpha);
void draw_item_no_int(Item *item, Oct_Vec2 position, float alpha);

// Draw an arbitrary object
void draw_object(ObjectInfo *object, Oct_Vec2 position, float scale, float alpha);
void draw_object_raw(ObjectInfo *info, Oct_Vec2 position, float scale, float alpha);
void draw_object_raw_no_int(ObjectInfo *info, Oct_Vec2 position, float scale, float alpha);

void create_slime(Character *slot, Position pos);
void create_zombie(Character *slot, Position pos);
void create_skeleton(Character *slot, Position pos);
