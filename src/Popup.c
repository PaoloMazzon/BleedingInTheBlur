#include <string.h>
#include "Game.h"
#include "Util.h"

#define PACK_POPUP_POINTER(generation, index) (((uint64_t)generation << 32) + (index))
#define UNPACK_GENERATION(pointer) ((pointer & (0xffffffff << 32)) >> 32)
#define UNPACK_INDEX(pointer) (pointer & 0xffffffff)

bool popups_are_active() {
    return g_game.current_level.popup_stack_pointer > 0;
}

void draw_and_update_popups() {
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

PopupDicePointer popup_dice(int32_t dc, int32_t pips) {
    if (g_game.current_level.popup_stack_pointer == MAX_POPUP_STACK)
        oct_Raise(OCT_STATUS_ERROR, true, "Dice popup was created on a full stack.");
    Popup *pop = &g_game.current_level.popup_stack[g_game.current_level.popup_stack_pointer++];
    pop->type = POPUP_TYPE_MESSAGE;
    pop->Dice.dc = dc;
    pop->Dice.pips = pips;

    return PACK_POPUP_POINTER(pop->generation, g_game.current_level.popup_stack_pointer - 1);
}

bool popup_get_input(PopupInputPointer input_pointer, const char *out) {
    return false; // TODO: This
}

bool popup_get_dice(PopupDicePointer dice_pointer, int32_t *out) {
    return false; // TODO: This
}
