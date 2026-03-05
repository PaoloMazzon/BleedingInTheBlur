/// \brief Random utilities
#pragma once
#include "Structs.h"

// Returns x if x is >= 0, otherwise it returns 0
int32_t non_negative(int32_t x);

// Returns a random int from [lower, upper)
int32_t random_int(int32_t lower, int32_t upper);

// Returns a new unique id for an object (starts at 1,000,000)
uint64_t new_oct_id();

// Updates camera coords, only call once per frame
void update_camera_coords();

// Returns the coordinates where the camera is supposed to be in the game world
void get_camera_coords(float *x, float *y, float *w, float *h);

// Tells the camera where it should be looking
void look_at(Position pos, float zoom);

// Simulates rolling dice, returns bool if the roll meets or beats the DC. out may be null
bool roll_dice(int32_t pips, int32_t dc, int32_t *result);
