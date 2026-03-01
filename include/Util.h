/// \brief Random utilities
#pragma once
#include "Structs.h"

// Returns x if x is >= 0, otherwise it returns 0
int32_t non_negative(int32_t x);

// Returns a random int from [lower, upper)
int32_t random_int(int32_t lower, int32_t upper);
