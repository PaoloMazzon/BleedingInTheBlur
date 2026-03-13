#include <string.h>
#include "Game.h"
#include "Util.h"

#define PACK_POPUP_POINTER(generation, index) (((uint64_t)generation << 32) + (index))
#define UNPACK_GENERATION(pointer) ((pointer & (0xffffffff << 32)) >> 32)
#define UNPACK_INDEX(pointer) (pointer & 0xffffffff)

bool popups_are_active() {
    return g_game.current_level.popup_stack_pointer > 0;
}

void draw_and_update_weapon_popup(Popup *weapon_popup) {
    // Show weapon pickup popup
    // 84, 84
}

void draw_and_update_popups() {
    const int32_t top_of_stack = g_game.current_level.popup_stack_pointer - 1;
    if (top_of_stack >= 0) {
        if (g_game.current_level.popup_stack[top_of_stack].type == POPUP_TYPE_WEAPON_SELECT) {
            draw_and_update_weapon_popup(&g_game.current_level.popup_stack[top_of_stack]);
        } else {
            oct_Raise(OCT_STATUS_ERROR, true, "Unimplemented popup type %i.", g_game.current_level.popup_stack[top_of_stack].type);
        }
    }
    // TODO: This
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
    return false; // TODO: This
}

bool popup_get_item(PopupItemSelectPointer item_pointer, int32_t *index) {
    return false; // TODO: This
}

