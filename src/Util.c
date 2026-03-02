#include <math.h>
#include "Util.h"
#include "Game.h"

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

void get_camera_coords(float *x, float *y) {
    *x = (g_game.player.info.actual_position[0] + (CELL_WIDTH / 2)) - (GAME_VIEW_WIDTH / 2);
    *y = (g_game.player.info.actual_position[1] + (CELL_HEIGHT / 2)) - (GAME_VIEW_HEIGHT / 2);
}
