/// \brief Handles attack animation setup and execution
#pragma once

// Top-level function to draw any attack animation
void draw_attack_animation();

// Top-level function to dish out damage as necessary depending on the animation type
void complete_attack_animation();

// Begins a melee attack animation
void setup_melee_animation(Character *attacker, Character *receiver, const Traits *attack_traits, bool passed, int32_t damage);