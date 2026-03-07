/// \brief Tools for level generation
#pragma once
#include "Structs.h"

// Generates a level in place, allocating everything necessary in level
void generate_level(Level *level, LevelGenerationParameters *parameters, Position out_player_pos);

// Cleans up anything made in generate_level
void cleanup_level(Level *level);