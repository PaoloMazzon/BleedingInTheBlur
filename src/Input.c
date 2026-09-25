#include "Input.h"
#include "Game.h"

void input_init(cJSON *load_from) {
    g_game.input_state.inputs = oct_Zalloc(g_game.allocator, sizeof(InputIndividualState) * INPUT_MAX);
    g_game.input_state.hold_delay = GAME_INPUT_HOLD_DELAY;

    // set inputs
    g_game.input_state.inputs[INPUT_LEFT].button_bind = OCT_GAMEPAD_BUTTON_DPAD_LEFT;
    g_game.input_state.inputs[INPUT_LEFT].key_bind = OCT_KEY_LEFT;
    g_game.input_state.inputs[INPUT_RIGHT].button_bind = OCT_GAMEPAD_BUTTON_DPAD_RIGHT;
    g_game.input_state.inputs[INPUT_RIGHT].key_bind = OCT_KEY_RIGHT;
    g_game.input_state.inputs[INPUT_UP].button_bind = OCT_GAMEPAD_BUTTON_DPAD_UP;
    g_game.input_state.inputs[INPUT_UP].key_bind = OCT_KEY_UP;
    g_game.input_state.inputs[INPUT_DOWN].button_bind = OCT_GAMEPAD_BUTTON_DPAD_DOWN;
    g_game.input_state.inputs[INPUT_DOWN].key_bind = OCT_KEY_DOWN;

    if (!load_from) return;
    const cJSON *delay = cJSON_GetObjectItem(load_from, "delay");
    if (delay && cJSON_IsNumber(delay)) {
        g_game.input_state.hold_delay = (float) cJSON_GetNumberValue(delay);
    }
}

void input_save(cJSON *serialize_into) {
    // Save to the key "input"
    if (!serialize_into) return;
    cJSON_AddNumberToObject(serialize_into, "delay", g_game.input_state.hold_delay);
}

void input_quit() {
    oct_Free(g_game.allocator, g_game.input_state.inputs);
}

void input_update() {
    for (int i = 0; i < INPUT_MAX; i++) {
        InputIndividualState *input = &g_game.input_state.inputs[i];
        if (oct_KeyPressed(input->key_bind) || oct_GamepadButtonPressed(0, input->button_bind)) {
            input->active = true;
            input->last_press = oct_Time();
        } else if (oct_KeyReleased(input->key_bind) || oct_GamepadButtonReleased(0, input->button_bind)) {
            input->active = false;
        }
    }
}

bool input_get(Input input) {
    InputIndividualState *state = &g_game.input_state.inputs[input];
    if (oct_KeyPressed(state->key_bind) || oct_GamepadButtonPressed(0, state->button_bind))
        return true;
    if (state->active && oct_Time() - state->last_press >= g_game.input_state.hold_delay)
        return true;
    return false;
}
