#include <math.h>
#include "Util.h"
#include "Game.h"

static float camera_x = 0;
static float camera_y = 0;
static float camera_zoom = 1;
static float camera_actual_zoom = 1;
static float camera_w = 160;
static float camera_h = 128;
static Position camera_lookat;
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
    camera_x += ((((float)camera_lookat[0] * CELL_WIDTH) + (CELL_WIDTH / 2)) - (camera_w / 2) - camera_x) * 0.2f;
    camera_y += ((((float)camera_lookat[1] * CELL_HEIGHT) + (CELL_HEIGHT / 2)) - (camera_h / 2) - camera_y) * 0.2f;
    camera_w = camera_actual_zoom * GAME_VIEW_WIDTH;
    camera_h = camera_actual_zoom * GAME_VIEW_HEIGHT;
    camera_actual_zoom += (camera_zoom - camera_actual_zoom) * 0.4f;
}

void get_camera_coords(float *x, float *y, float *w, float *h) {
    if (x) *x = camera_x;
    if (y) *y = camera_y;
    if (w) *w = camera_w;
    if (h) *h = camera_h;
}

void look_at(Position pos, float zoom) {
    camera_lookat[0] = pos[0];
    camera_lookat[1] = pos[1];
    camera_zoom = zoom;
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

int32_t tile_distance(Position p1, Position p2) {
    const int32_t x_delta = abs(p1[0] - p2[0]);
    const int32_t y_delta = abs(p1[1] - p2[1]);
    return x_delta + y_delta;
}
