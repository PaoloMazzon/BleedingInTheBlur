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

// Simulates rolling ups on dice. This means that it rolls 1d8 and a number of d6 equal to the
// pips but the roll is only considered successful if the max value is rolled on at least
// up_count of the dice.
bool roll_ups(int32_t pips, int32_t up_count, int32_t *out_rolled_ups);

// returns the manhattan distance between two tiles
int32_t tile_distance(Position p1, Position p2);

// for an inverse parabola normalized to [0, 1]
float hyperbolic_x(float normalized_x);

// for managing timers, once timer_tick is called on a complete timer the timer is no longer valid
void timer_start(Timer *timer, int32_t duration_in_frames);
bool timer_tick(Timer *timer); // does nothing if the timer is not set, returns true if this frame it goes off
float timer_get_normalized(Timer *timer); // returns a float from 0-1 representing how much time is left where 1 is full time
bool timer_is_done(Timer *timer); // returns true if the timer is done, only valid until timer_tick is called
bool timer_in_use(Timer *timer); // returns true if the timer is currently in use
int32_t timer_frames_left(Timer *timer);

// Basic vector math - these always return the "out" variable where applicable
int32_t *add_pos(const Position lhs, const Position rhs, Position out);
int32_t *subtract_pos(const Position lhs, const Position rhs, Position out);
int32_t dot_pos(const Position lhs, const Position rhs);
float *add_vec2(const Oct_Vec2 lhs, const Oct_Vec2 rhs, Oct_Vec2 out);
float *subtract_vec2(const Oct_Vec2 lhs, const Oct_Vec2 rhs, Oct_Vec2 out);
float dot_vec2(const Oct_Vec2 lhs, const Oct_Vec2 rhs);
