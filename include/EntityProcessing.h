/// \brief High-level character processing for levels, since it deals in
/// many characters it doesn't really belong in Characters.h
#pragma once

// Update the logic for all characters
bool characters_update();

// Draw all characters, including any on-going attacks
void draw_characters();

// Outlines are for when the player is aware of a character but can't physically see them
void draw_character_outlines();
