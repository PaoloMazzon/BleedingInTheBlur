#include "MenuDetails.h"
#include "Game.h"

#define start_y 90
#define start_x 20
#define movement_y 20
const MenuOption music_volume_option = {
    .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
    .name = "Music Volume",
    .max_index = 10,
    .drawn_position = {start_x, start_y + (movement_y * 0)},
    .change_callback = music_volume_change_callback,
    .draw_callback = music_volume_draw_callback,
};
const MenuOption sfx_volume_option = {
    .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
    .name = "SFX Volume",
    .max_index = 10,
    .drawn_position = {start_x, start_y + (movement_y * 1)},
    .change_callback = sfx_volume_change_callback,
    .draw_callback = sfx_volume_draw_callback,
};
const MenuOption animation_speed_option = {
    .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
    .name = "Animation Speed",
    .max_index = ANIMATION_SPEED_MAX,
    .drawn_position = {start_x, start_y + (movement_y * 2)},
    .change_callback = animation_speed_change_callback,
    .draw_callback = animation_speed_draw_callback,
};
const MenuOption auto_pick_up_item_option = {
    .type = MENU_OPTION_TYPE_SELECT,
    .name = "Auto-Pickup",
    .drawn_position = {start_x, start_y + (movement_y * 3)},
    .change_callback = auto_pick_up_item_change_callback,
    .draw_callback = auto_pick_up_item_draw_callback,
};
const MenuOption animate_enemy_movement_option = {
    .type = MENU_OPTION_TYPE_SELECT,
    .name = "Animate Enemy Movement",
    .drawn_position = {start_x, start_y + (movement_y * 4)},
    .change_callback = animate_enemy_movement_change_callback,
    .draw_callback = animate_enemy_movement_draw_callback,
};
const MenuOption fullscreen_option = {
    .type = MENU_OPTION_TYPE_SELECT,
    .name = "Fullscreen",
    .drawn_position = {start_x, start_y + (movement_y * 5)},
    .change_callback = fullscreen_change_callback,
    .draw_callback = fullscreen_draw_callback,
};
const MenuOption scale_mode_option = {
    .type = MENU_OPTION_TYPE_CYCLE_HORIZONTAL,
    .name = "Scale Mode",
    .max_index = SCALE_MODE_MAX,
    .drawn_position = {start_x, start_y + (movement_y * 6)},
    .change_callback = scale_mode_change_callback,
    .draw_callback = scale_mode_draw_callback,
};

void music_volume_change_callback(int32_t index) {
    // TODO: This
}
void music_volume_draw_callback(Oct_Vec2 position, int32_t index) {
    // TODO: This
}

void sfx_volume_change_callback(int32_t index) {
    // TODO: This
}
void sfx_volume_draw_callback(Oct_Vec2 position, int32_t index) {
    // TODO: This
}

void animation_speed_change_callback(int32_t index) {
    // TODO: This
}
void animation_speed_draw_callback(Oct_Vec2 position, int32_t index) {
    // TODO: This
}

void auto_pick_up_item_change_callback(int32_t index) {
    // TODO: This
}
void auto_pick_up_item_draw_callback(Oct_Vec2 position, int32_t index) {
    // TODO: This
}

void animate_enemy_movement_change_callback(int32_t index) {
    // TODO: This
}
void animate_enemy_movement_draw_callback(Oct_Vec2 position, int32_t index) {
    // TODO: This
}

void fullscreen_change_callback(int32_t index) {
    // TODO: This
}
void fullscreen_draw_callback(Oct_Vec2 position, int32_t index) {
    // TODO: This
}

void scale_mode_change_callback(int32_t index) {
    // TODO: This
}
void scale_mode_draw_callback(Oct_Vec2 position, int32_t index) {
    // TODO: This
}
