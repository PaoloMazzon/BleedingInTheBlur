#include <math.h>
#include "Util.h"
#include "Game.h"

static float camera_x = 0;
static float camera_y = 0;
static uint64_t object_id_counter = 1000000;

int32_t non_negative(int32_t x) {
    return x < 0 ? 0 : x;
}

int32_t random_int(int32_t lower, int32_t upper) {
    const float r = oct_Random((float)lower, (float)upper - 0.01f);
    return (int32_t)floorf(r);
}

uint64_t new_oct_id() {
    return object_id_counter++;
}

void update_camera_coords() {
    camera_x += ((g_game.player.info.actual_position[0] + (CELL_WIDTH / 2)) - (GAME_VIEW_WIDTH / 2) - camera_x) * 0.4f;
    camera_y += ((g_game.player.info.actual_position[1] + (CELL_HEIGHT / 2)) - (GAME_VIEW_HEIGHT / 2) - camera_y) * 0.4f;
}

void get_camera_coords(float *x, float *y) {
    *x = camera_x;
    *y = camera_y;
}

bool roll_dice(int32_t pips, int32_t dc, int32_t *result) {
    int32_t sum = random_int(1, 9);
    if (sum == 8)
        sum += 4;
    for (int i = 0; i < pips; i++) {
        sum += random_int(1, 7);
    }
    if (result)
        *result = sum;
    return sum >= dc;
}
