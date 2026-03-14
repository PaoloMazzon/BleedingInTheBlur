#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "Structs.h"
#include "Character.h"
#include "Game.h"
#include "Util.h"

#define PACK_POPUP_POINTER(generation, index) (((uint64_t)generation << 32) + (index))
#define UNPACK_GENERATION(pointer) ((pointer & (0xffffffff00000000)) >> 32)
#define UNPACK_INDEX(pointer) (pointer & 0xffffffff)

bool popups_are_active() {
    return g_game.current_level.popup_stack_pointer > 0;
}

bool draw_and_update_weapon_popup(Popup *weapon_popup) {
    static const int32_t buffer_size = 50;
    static char new_weapon_buffer[51];
    static char old_weapon_buffer[51];
    const float start_x = 84;
    const float start_y = 84;
    oct_DrawTexture(oct_GetAsset(g_game.assets, "hud/newweaponpopup.png"), (Oct_Vec2){start_x, start_y});

    // Tween pointer rotation and position
    float target_rotation = 0;
    Oct_Vec2 target_position = {0};
    if (weapon_popup->Weapon.selected_yes) {
        target_position[0] = start_x + 90;
        target_position[1] = start_y + 44;
        target_rotation = 0;
    } else {
        target_position[0] = start_x + 61;
        target_position[1] = start_y + 43;
        target_rotation = 3.141592635f;
    }
    weapon_popup->Weapon.actual_pointer_pos[0] += (target_position[0] - weapon_popup->Weapon.actual_pointer_pos[0]) * 0.4f;
    weapon_popup->Weapon.actual_pointer_pos[1] += (target_position[1] - weapon_popup->Weapon.actual_pointer_pos[1]) * 0.4f;
    weapon_popup->Weapon.actual_pointer_rotation += (target_rotation - weapon_popup->Weapon.actual_pointer_rotation) * 0.4f;

    // Draw pointer
    oct_DrawTextureIntExt(
            OCT_INTERPOLATE_ALL, POPUP_POINTER_ID,
            oct_GetAsset(g_game.assets, "hud/selectionarrow.png"),
            weapon_popup->Weapon.actual_pointer_pos,
            (Oct_Vec2){1, 1},
            weapon_popup->Weapon.actual_pointer_rotation, (Oct_Vec2){OCT_ORIGIN_MIDDLE, OCT_ORIGIN_MIDDLE});


    // We are going to draw 4 pieces of text, so get all their metrics now
    Oct_Vec2 new_weapon_name_size;
    Oct_Vec2 old_weapon_name_size;
    int32_t old_weapon_dc, new_weapon_dc, old_weapon_pips, new_weapon_pips;
    character_get_attack_base_stats(&g_game.player, &weapon_popup->Weapon.weapon->info.traits, &new_weapon_pips, &new_weapon_dc);
    character_get_attack_base_stats(&g_game.player, &g_game.player.soul_bound_weapon.info.traits, &old_weapon_pips, &old_weapon_dc);
    snprintf(new_weapon_buffer, buffer_size, "1%s%i%s%s%i", GLYPH_D8, new_weapon_pips, GLYPH_D6, GLYPH_ARROW, new_weapon_dc);
    snprintf(old_weapon_buffer, buffer_size, "1%s%i%s%s%i", GLYPH_D8, old_weapon_pips, GLYPH_D6, GLYPH_ARROW, old_weapon_dc);
    float new_weapon_stats_size = (float)strlen(new_weapon_buffer) * 6.0f;
    float old_weapon_stats_size = (float)strlen(old_weapon_buffer) * 6.0f;
    const Oct_FontAtlas pretty_font = oct_GetAsset(g_game.assets, "fnt_pixel");
    const Oct_FontAtlas dice_font = oct_GetAsset(g_game.assets, "fnt_dice");
    oct_GetTextSize(pretty_font, new_weapon_name_size, 1, "%s", weapon_popup->Weapon.weapon->info.name);
    oct_GetTextSize(pretty_font, old_weapon_name_size, 1, "%s", g_game.player.soul_bound_weapon.info.name);

    // Actually draw the text
    oct_DrawText(pretty_font,
                 (Oct_Vec2){start_x + 35 - (new_weapon_name_size[0] / 2), start_y + 18 - (new_weapon_name_size[1] / 2)},
                 1,
                 weapon_popup->Weapon.weapon->info.name);
    oct_DrawText(pretty_font,
                 (Oct_Vec2){start_x + 115 - (new_weapon_name_size[0] / 2), start_y + 18 - (new_weapon_name_size[1] / 2)},
                 1,
                 g_game.player.soul_bound_weapon.info.name);
    oct_DrawText(dice_font,
                 (Oct_Vec2){start_x + 35 - (new_weapon_stats_size / 2), start_y + 60},
                 1,
                 new_weapon_buffer);
    oct_DrawText(dice_font,
                 (Oct_Vec2){start_x + 115 - (new_weapon_stats_size / 2), start_y + 60},
                 1,
                 old_weapon_buffer);

    // Draw the weapon icons
    oct_DrawTexture(
            weapon_popup->Weapon.weapon->icon,
            (Oct_Vec2){start_x + 28, start_y + 36});
    if (g_game.player.soul_bound_weapon.type != WEAPON_TYPE_NONE)
        oct_DrawTexture(
                g_game.player.soul_bound_weapon.icon,
                (Oct_Vec2){start_x + 108, start_y + 36});

    // Handle the controls
    if (oct_KeyPressed(BUTTON_LEFT) || oct_KeyPressed(BUTTON_RIGHT))
        weapon_popup->Weapon.selected_yes = !weapon_popup->Weapon.selected_yes;
    if (oct_KeyPressed(BUTTON_CONFIRM)) {
        weapon_popup->value_available = true;
        return true;
    }
    return false;
}

void draw_and_update_popups() {
    const int32_t top_of_stack = g_game.current_level.popup_stack_pointer - 1;
    if (top_of_stack >= 0) {
        bool pop_stack = false;
        if (g_game.current_level.popup_stack[top_of_stack].type == POPUP_TYPE_WEAPON_SELECT) {
            pop_stack = draw_and_update_weapon_popup(&g_game.current_level.popup_stack[top_of_stack]);
        } else {
            oct_Raise(OCT_STATUS_ERROR, true, "Unimplemented popup type %i.", g_game.current_level.popup_stack[top_of_stack].type);
        }
        if (pop_stack) {
            g_game.current_level.popup_stack_pointer -= 1;
        }
    }
}

void popup_message(const char *text, bool needs_to_be_freed) {
    if (g_game.current_level.popup_stack_pointer == MAX_POPUP_STACK)
        oct_Raise(OCT_STATUS_ERROR, true, "Message popup was created on a full stack.");
    Popup *pop = &g_game.current_level.popup_stack[g_game.current_level.popup_stack_pointer++];
    pop->type = POPUP_TYPE_MESSAGE;
    pop->Message.needs_to_be_freed = needs_to_be_freed;
    pop->Message.message = text;
}

PopupInputPointer popup_input(const char *text, bool needs_to_be_freed) {
    if (g_game.current_level.popup_stack_pointer == MAX_POPUP_STACK)
        oct_Raise(OCT_STATUS_ERROR, true, "Input popup was created on a full stack.");
    Popup *pop = &g_game.current_level.popup_stack[g_game.current_level.popup_stack_pointer++];
    pop->type = POPUP_TYPE_TEXT_INPUT;
    pop->TextInput.needs_to_be_freed = needs_to_be_freed;
    pop->TextInput.message = text;
    memset(pop->TextInput.user_input, 0, MAX_USER_INPUT_SIZE);

    return PACK_POPUP_POINTER(pop->generation, g_game.current_level.popup_stack_pointer - 1);
}


PopupWeaponSelectPointer popup_weapon_select(Weapon *weapon) {
    if (g_game.current_level.popup_stack_pointer == MAX_POPUP_STACK)
            oct_Raise(OCT_STATUS_ERROR, true, "Input popup was created on a full stack.");
    Popup *pop = &g_game.current_level.popup_stack[g_game.current_level.popup_stack_pointer++];
    pop->type = POPUP_TYPE_WEAPON_SELECT;
    pop->Weapon.weapon = weapon;

    return PACK_POPUP_POINTER(pop->generation, g_game.current_level.popup_stack_pointer - 1);
}

PopupItemSelectPointer popup_item_select(Item *item) {
    if (g_game.current_level.popup_stack_pointer == MAX_POPUP_STACK)
            oct_Raise(OCT_STATUS_ERROR, true, "Input popup was created on a full stack.");
    Popup *pop = &g_game.current_level.popup_stack[g_game.current_level.popup_stack_pointer++];
    pop->type = POPUP_TYPE_ITEM_SELECT;
    pop->Weapon.weapon = item;

    return PACK_POPUP_POINTER(pop->generation, g_game.current_level.popup_stack_pointer - 1);
}

bool popup_get_input(PopupInputPointer input_pointer, const char *out) {
    return false; // TODO: This
}

bool popup_get_weapon(PopupWeaponSelectPointer weapon_pointer, bool *selected) {
    const uint64_t index = UNPACK_INDEX(weapon_pointer);
    const uint64_t generation = UNPACK_GENERATION(weapon_pointer);
    assert(index >= 0);
    assert(index < MAX_POPUP_STACK);
    if (generation == g_game.current_level.popup_stack[index].generation && g_game.current_level.popup_stack[index].value_available) {
        g_game.current_level.popup_stack[index].generation++;
        g_game.current_level.popup_stack[index].value_available = false;
        *selected = g_game.current_level.popup_stack[index].Weapon.selected_yes;
        return true;
    }
    return false;
}

bool popup_get_item(PopupItemSelectPointer item_pointer, int32_t *index) {
    return false; // TODO: This
}

