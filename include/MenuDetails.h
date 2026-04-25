/// \brief Reusable menu callbacks. Think things like options
#include <stdint.h>
#include "Structs.h"

// Options that can be changed
void music_volume_change_callback(int32_t index);
void music_volume_draw_callback(Oct_Vec2 position, int32_t index);

void sfx_volume_change_callback(int32_t index);
void sfx_volume_draw_callback(Oct_Vec2 position, int32_t index);

void animation_speed_change_callback(int32_t index);
void animation_speed_draw_callback(Oct_Vec2 position, int32_t index);

void auto_pick_up_item_change_callback(int32_t index);
void auto_pick_up_item_draw_callback(Oct_Vec2 position, int32_t index);

void animate_enemy_movement_change_callback(int32_t index);
void animate_enemy_movement_draw_callback(Oct_Vec2 position, int32_t index);

void fullscreen_change_callback(int32_t index);
void fullscreen_draw_callback(Oct_Vec2 position, int32_t index);

void scale_mode_change_callback(int32_t index);
void scale_mode_draw_callback(Oct_Vec2 position, int32_t index);

extern const MenuOption music_volume_option;
extern const MenuOption sfx_volume_option;
extern const MenuOption animation_speed_option;
extern const MenuOption auto_pick_up_item_option;
extern const MenuOption animate_enemy_movement_option;
extern const MenuOption fullscreen_option;
extern const MenuOption scale_mode_option;
