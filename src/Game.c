#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <oct/cJSON.h>
#include <slog.h>
#include "Game.h"
#include "Structs.h"

Game g_game;
static FILE *g_log_file = NULL;

// Loads all the sprites in a given path using evil string manipulation
// Always allocates 1 extra spot for a blank sprite
void load_sprite_options(const char *path, int32_t count, Oct_Sprite **dest_array) {
    *dest_array = oct_Malloc(g_game.allocator, sizeof(Oct_Sprite) * (count + 1));
    Oct_Sprite *array = *dest_array;
    array[0] = OCT_NO_ASSET;
    char temp_path[256];
    strncpy(temp_path, path, 255);
    const int32_t evil_index = strlen(temp_path) - 6;
    for (int32_t i = 0; i < count; i++) {
        temp_path[evil_index] = (char)(i + 48);
        array[i + 1] = oct_GetAsset(g_game.assets, temp_path);
    }
}

// Loads the customizable character options -- this is a pile of shit
void load_sprite_layers() {
    Oct_Colour c0 = {.r = 0.94f, .g = 0.94f, .b = 0.94f, .a = 1.0f};
    g_game.layer_colours[0] = c0;
    Oct_Colour c1 = {.r = 0.27f, .g = 0.27f, .b = 0.27f, .a = 1.0f};
    g_game.layer_colours[1] = c1;
    Oct_Colour c2 = {.r = 0.04f, .g = 0.15f, .b = 0.32f, .a = 1.0f};
    g_game.layer_colours[2] = c2;
    Oct_Colour c3 = {.r = 0.07f, .g = 0.04f, .b = 0.32f, .a = 1.0f};
    g_game.layer_colours[3] = c3;
    Oct_Colour c4 = {.r = 0.32f, .g = 0.04f, .b = 0.30f, .a = 1.0f};
    g_game.layer_colours[4] = c4;
    Oct_Colour c5 = {.r = 0.32f, .g = 0.04f, .b = 0.09f, .a = 1.0f};
    g_game.layer_colours[5] = c5;
    Oct_Colour c6 = {.r = 0.04f, .g = 0.32f, .b = 0.15f, .a = 1.0f};
    g_game.layer_colours[6] = c6;
    Oct_Colour c7 = {.r = 0.15f, .g = 0.32f, .b = 0.04f, .a = 1.0f};
    g_game.layer_colours[7] = c7;
    Oct_Colour c8 = {.r = 0.32f, .g = 0.31f, .b = 0.04f, .a = 1.0f};
    g_game.layer_colours[8] = c8;
    Oct_Colour c9 = {.r = 0.32f, .g = 0.15f, .b = 0.04f, .a = 1.0f};
    g_game.layer_colours[9] = c9;

    g_game.layer_sprite_counts[0] = 10;
    load_sprite_options("characterbodylayer/option_0.json", g_game.layer_sprite_counts[0], &g_game.layers_sprites[0]);
    g_game.layer_sprite_counts[0]++;
    g_game.layer_sprite_counts[1] = 2;
    load_sprite_options("characterpantslayer/option_0.json", g_game.layer_sprite_counts[1], &g_game.layers_sprites[1]);
    g_game.layer_sprite_counts[1]++;
    g_game.layer_sprite_counts[2] = 1;
    load_sprite_options("charactershoelayer/option_0.json", g_game.layer_sprite_counts[2], &g_game.layers_sprites[2]);
    g_game.layer_sprite_counts[2]++;
    g_game.layer_sprite_counts[3] = 4;
    load_sprite_options("charactershirtlayer/option_0.json", g_game.layer_sprite_counts[3], &g_game.layers_sprites[3]);
    g_game.layer_sprite_counts[3]++;
    g_game.layer_sprite_counts[4] = 8;
    load_sprite_options("charactertoplayer/option_0.json", g_game.layer_sprite_counts[4], &g_game.layers_sprites[4]);
    g_game.layer_sprite_counts[4]++;
    g_game.layer_sprite_counts[5] = 10;
    load_sprite_options("characteraccessorylayer/option_0.json", g_game.layer_sprite_counts[5], &g_game.layers_sprites[5]);
    g_game.layer_sprite_counts[5]++;
}

static void draw_level_transitions() {
    const float frames_remaining = g_game.level_transitions.duration - (g_game.frame - g_game.level_transitions.start_frame);
    const float percent_remaining = frames_remaining / g_game.level_transitions.duration;

    if (g_game.level_transitions.type == TRANSITION_TYPE_FADE_OUT) {
        const Oct_Vec2 position = {-368.0f + ((1.0f - percent_remaining) * 368.0f)};
        oct_DrawTexture(
                oct_GetAsset(g_game.assets, "menu/fadeeffect.png"),
                position);
    } else if (g_game.level_transitions.type == TRANSITION_TYPE_FADE_IN) {
        oct_DrawTexture(
                oct_GetAsset(g_game.assets, "menu/fadeeffect.png"),
                (Oct_Vec2){-368 + (percent_remaining * 368)});
    } else {
        slog_warn("Unimplemented level transition %i", g_game.level_transitions.type);
    }
}

// Calls a level's update function
static void level_update_function(LevelIndex level) {
    switch (level) {
        case LEVEL_INDEX_MENU:
            menu_update();
            break;
        case LEVEL_INDEX_FLOOR_1:
        case LEVEL_INDEX_FLOOR_2:
        case LEVEL_INDEX_FLOOR_3:
        case LEVEL_INDEX_FLOOR_4:
        case LEVEL_INDEX_FLOOR_5:
            level_update();
            break;
        default:
            slog_fatal("Unhandled level index %i.", level);
    }
}

// Calls a level's end function
static void level_end_function(LevelIndex level) {
    slog_debug("Calling level end function for %s", LEVEL_INDEX_NAMES[level]);
    switch (level) {
        case LEVEL_INDEX_MENU:
            menu_end();
            break;
        case LEVEL_INDEX_FLOOR_1:
        case LEVEL_INDEX_FLOOR_2:
        case LEVEL_INDEX_FLOOR_3:
        case LEVEL_INDEX_FLOOR_4:
        case LEVEL_INDEX_FLOOR_5:
            level_end();
            break;
        default:
            slog_fatal("Unhandled level index %i.", level);
    }
}

// Calls a level's start function
static void level_begin_function(LevelIndex level) {
    slog_debug("Calling level begin function for %s", LEVEL_INDEX_NAMES[level]);
    switch (level) {
        case LEVEL_INDEX_MENU:
            menu_begin();
            break;
        case LEVEL_INDEX_FLOOR_1:
        case LEVEL_INDEX_FLOOR_2:
        case LEVEL_INDEX_FLOOR_3:
        case LEVEL_INDEX_FLOOR_4:
        case LEVEL_INDEX_FLOOR_5:
            level_begin();
            break;
        default:
            slog_fatal("Unhandled level index %i.", level);
    }
}



static size_t strip_ansi(const char *pIn, size_t nLen, char *pOut, size_t nOutSize)
{
    size_t i = 0, j = 0;
    while (i < nLen && j + 1 < nOutSize)
    {
        if (pIn[i] == '\x1B' && i + 1 < nLen && pIn[i + 1] == '[')
        {
            i += 2;
            while (i < nLen && pIn[i] != 'm') i++;
            if (i < nLen) i++; /* skip the 'm' */
            continue;
        }
        pOut[j++] = pIn[i++];
    }
    pOut[j] = '\0';
    return j;
}

static int log_to_file_cb(const char *pLog, size_t nLength, slog_flag_t eFlag, void *pCtx)
{
    (void)eFlag; (void)pCtx;
    if (g_log_file != NULL)
    {
        char sPlain[SLOG_MESSAGE_MAX];
        strip_ansi(pLog, nLength, sPlain, sizeof(sPlain));
        fputs(sPlain, g_log_file);
        fflush(g_log_file);
    }
    return 1; /* 1 = let slog also print (colored) to screen as usual */
}

void setup_logging() {
    uint16_t enabled_level = SLOG_FATAL | SLOG_ERROR | SLOG_WARN | SLOG_INFO | SLOG_NOTAG;
#ifndef NDEBUG
    const bool debug = true;
#else
    const bool debug = g_game.options.enable_debug_logs;
#endif
    if (debug)
        enabled_level |= SLOG_DEBUG;
    slog_init("bleeding_in_the_blur", enabled_level, false);
    g_log_file = fopen("bleeding_in_the_blur.log", "a");
    slog_callback_set(log_to_file_cb, NULL);
    slog_config_t config;
    slog_config_get(&config);
    config.eDateControl = SLOG_DATE_FULL;
    config.nKeepOpen = 1; // Keep file handle open for next file writes
    config.nUseHeap = 1; // Use dynamic allocation
    config.nToFile = 1; // Enable file logging
    config.nRotate = 1; // Enable log rotation
    config.nFlush = 1; // Flush stdout after screen log
    slog_config_set(&config);
}

void *startup() {
    slog_debug("Starting the game.");

    // Octarine things
    g_game.backbuffer = oct_CreateSurface((Oct_Vec2){VIRTUAL_WIDTH, VIRTUAL_HEIGHT});
    g_game.world_camera = oct_CreateCamera();
    g_game.ui_camera = oct_CreateCamera();
    g_game.render_camera = oct_CreateCamera();
    g_game.assets = oct_LoadAssetBundle("data");
    g_game.allocator = oct_CreateHeapAllocator();
    load_options();
    setup_logging();
    oct_SetFullscreen(g_game.options.fullscreen);

    // Setup
    const float window_width = oct_WindowWidth();
    const float window_height = oct_WindowHeight();
    Oct_CameraUpdate world_camera_update = {
            .size = {GAME_VIEW_WIDTH, GAME_VIEW_HEIGHT},
            .screenSize = {VIRTUAL_WIDTH, VIRTUAL_HEIGHT},
    };
    Oct_CameraUpdate ui_camera_update = {
            .size = {VIRTUAL_WIDTH, VIRTUAL_HEIGHT},
            .screenSize = {VIRTUAL_WIDTH, VIRTUAL_HEIGHT},
    };
    oct_UpdateCamera(g_game.world_camera, &world_camera_update);
    oct_UpdateCamera(g_game.ui_camera, &ui_camera_update);
    g_game.frame = 1;

    load_sprite_layers();
    menu_begin();
    return nullptr;
}

void *update(void *ptr) {
#ifndef NDEBUG
    static bool debug_mode = true;
#else
    static bool debug_mode = false;
#endif
    oct_SetDrawTarget(g_game.backbuffer);
    oct_DrawClear(&(Oct_Colour){.r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f});
    oct_SetTextureCamerasEnabled(true);

    // Update current level
    level_update_function(g_game.level_index);

    // Update level transitions
    if (in_level_transition()) {
        draw_level_transitions();

        // Transition between levels
        if (g_game.frame >= (g_game.level_transitions.start_frame + g_game.level_transitions.duration)) {
            if (g_game.level_transitions.next_level != LEVEL_INDEX_NONE) {
                level_end_function(g_game.level_index);
                level_begin_function(g_game.level_transitions.next_level);
                g_game.level_index = g_game.level_transitions.next_level;
            }
            g_game.level_transitions.in_transition = false;

            if (g_game.level_transitions.type == TRANSITION_TYPE_FADE_OUT) {
                queue_level_transition(LEVEL_INDEX_NONE, TRANSITION_TYPE_FADE_IN, g_game.level_transitions.duration);
            }
        }
    }

    // Toggle fullscreen
    if (oct_KeyDown(OCT_KEY_LALT) && oct_KeyPressed(OCT_KEY_RETURN)) {
        g_game.options.fullscreen = !g_game.options.fullscreen;
        oct_SetFullscreen(g_game.options.fullscreen);
        save_options();
    }
    if (oct_KeyPressed(OCT_KEY_1)) {
        g_game.options.scale_mode = SCALE_MODE_INTEGER;
        save_options();
    }
    if (oct_KeyPressed(OCT_KEY_2)) {
        g_game.options.scale_mode = SCALE_MODE_ASPECT_RATIO;
        save_options();
    }
    if (oct_KeyPressed(OCT_KEY_3)) {
        g_game.options.scale_mode = SCALE_MODE_STRETCH;
        save_options();
    }
    if (oct_KeyPressed(OCT_KEY_F1))
        debug_mode = !debug_mode;
    const float window_width = oct_WindowWidth();
    const float window_height = oct_WindowHeight();

    Oct_CameraUpdate camera_update = {
            .size = {window_width, window_height},
            .screenSize = {window_width, window_height},
    };
    oct_UpdateCamera(g_game.render_camera, &camera_update);

    // Draw from the backbuffer to the screen
    oct_SetDrawTarget(OCT_TARGET_SWAPCHAIN);
    oct_SetTextureCamerasEnabled(false);
    oct_LockCameras(g_game.render_camera);
    oct_DrawClear(&(Oct_Colour){.r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f});
    if (g_game.options.scale_mode == SCALE_MODE_INTEGER) {
        const float scale_x = floorf(window_width / VIRTUAL_WIDTH);
        const float scale_y = floorf(window_height / VIRTUAL_HEIGHT);
        const float scale = scale_x < scale_y ? scale_x : scale_y;
        const float x_offset = (window_width - (VIRTUAL_WIDTH * scale)) * 0.5f;
        const float y_offset = (window_height - (VIRTUAL_HEIGHT * scale)) * 0.5f;
        oct_DrawTextureExt(
                g_game.backbuffer,
                (Oct_Vec2) {x_offset, y_offset},
                (Oct_Vec2) {scale, scale},
                0, (Oct_Vec2) {0, 0});
    } else if (g_game.options.scale_mode == SCALE_MODE_ASPECT_RATIO) {
        const float scale_x = window_width / VIRTUAL_WIDTH;
        const float scale_y = window_height / VIRTUAL_HEIGHT;
        const float scale = scale_x < scale_y ? scale_x : scale_y;
        const float x_offset = (window_width - (VIRTUAL_WIDTH * scale)) * 0.5f;
        const float y_offset = (window_height - (VIRTUAL_HEIGHT * scale)) * 0.5f;
        oct_DrawTextureExt(
                g_game.backbuffer,
                (Oct_Vec2) {x_offset, y_offset},
                (Oct_Vec2) {scale, scale},
                0, (Oct_Vec2) {0, 0});
    } else /* Stretch */ {
        oct_DrawTextureExt(
                g_game.backbuffer,
                (Oct_Vec2){0, 0},
                (Oct_Vec2){window_width / VIRTUAL_WIDTH, window_height / VIRTUAL_HEIGHT},
                0, (Oct_Vec2){0, 0});
    }

    if (debug_mode) {
        // Draw debug info
        oct_DrawText(oct_GetAsset(g_game.assets, "fnt_small"),
                     (Oct_Vec2) {1, 0}, 1,
                     "Player: %i, %i\nState: %s\nAttack animation: %s\nLogic: %.2fHz Refresh: %.2ffps\nPopup stack pointer: %i\nTurn: %i",
                     g_game.player.pos[0], g_game.player.pos[1],
                     GAME_STATES[g_game.current_level.state],
                     level_in_attack_animation() ? "yes" : "no",
                     oct_GetLogicHz(),
                     oct_GetRenderFPS(),
                     g_game.current_level.popup_stack_pointer,
                     g_game.current_level.turn);
    }

    g_game.frame++;
    return nullptr;
}

void shutdown(void *ptr) {
    save_options();
    oct_FreeAssetBundle(g_game.assets);
    oct_FreeAllocator(g_game.allocator);
}

void set_draw_target(Oct_Texture tex) {
    oct_SetDrawTarget(tex);
    oct_SetTextureCamerasEnabled(false);
}

void reset_draw_target() {
    oct_SetDrawTarget(g_game.backbuffer);
    oct_SetTextureCamerasEnabled(true);
}

static const Options default_options_struct = {
    .music_volume = 1,
    .sfx_volume = 1,
    .animation_speed = ANIMATION_SPEED_FULL,
    .auto_pick_up_item = false,
    .animate_enemy_movement = true,
    .scale_mode = SCALE_MODE_INTEGER,
    .fullscreen = true,
    .enable_debug_logs = false,
};

// json should be a json object
static double get_json_number_with_default(cJSON *json, const char *name, double default_val) {
    cJSON *nested = cJSON_GetObjectItem(json, name);
    if (!nested) return default_val;
    return cJSON_GetNumberValue(nested);
}

// json should be a json object
static bool get_json_bool_with_default(cJSON *json, const char *name, bool default_val) {
    cJSON *nested = cJSON_GetObjectItem(json, name);
    if (!nested) return default_val;
    return cJSON_IsTrue(nested);
}

void load_options() {
    uint32_t length;
    void *json_buffer = oct_ReadFile("save.json", g_game.allocator, &length);
    if (!json_buffer) {
        slog_warn("Failed to open save file for reading");
        default_options();
        return;
    }
    cJSON *json = cJSON_ParseWithLength(json_buffer, length);

    g_game.options.music_volume = (float)get_json_number_with_default(json, "music", default_options_struct.music_volume);
    g_game.options.sfx_volume = (float)get_json_number_with_default(json, "sound", default_options_struct.sfx_volume);
    g_game.options.animation_speed = (AnimationSpeed)get_json_number_with_default(json, "animation speed", default_options_struct.animation_speed);
    g_game.options.auto_pick_up_item = get_json_bool_with_default(json, "auto pickup", default_options_struct.auto_pick_up_item);
    g_game.options.animate_enemy_movement = get_json_bool_with_default(json, "animate enemy movement", default_options_struct.animate_enemy_movement);
    g_game.options.fullscreen = get_json_bool_with_default(json, "fullscreen", default_options_struct.fullscreen);
    g_game.options.scale_mode = (ScaleMode)get_json_number_with_default(json, "scale mode", default_options_struct.scale_mode);
    g_game.options.enable_debug_logs = (bool)get_json_bool_with_default(json, "debug logs", default_options_struct.enable_debug_logs);

    cJSON_Delete(json);
    oct_Free(g_game.allocator, json_buffer);
}

void default_options() {
    memcpy(&g_game.options, &default_options_struct, sizeof(Options));
}

void save_options() {
    cJSON *json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "music", g_game.options.music_volume);
    cJSON_AddNumberToObject(json, "sound", g_game.options.sfx_volume);
    cJSON_AddNumberToObject(json, "animation speed", g_game.options.animation_speed);
    cJSON_AddBoolToObject(json, "auto pickup", g_game.options.auto_pick_up_item);
    cJSON_AddBoolToObject(json, "animate enemy movement", g_game.options.animate_enemy_movement);
    cJSON_AddBoolToObject(json, "fullscreen", g_game.options.fullscreen);
    cJSON_AddNumberToObject(json, "scale mode", g_game.options.scale_mode);
    cJSON_AddBoolToObject(json, "debug logs", g_game.options.enable_debug_logs);

    FILE *f = fopen("save.json", "w");
    if (f) {
        char *text = cJSON_PrintUnformatted(json);
        fprintf(f, "%s", text);
        free(text);
        fclose(f);
    } else {
        slog_warn("Failed to open save file for writing");
    }
    cJSON_Delete(json);
}

int32_t get_options_attack_duration() {
    switch (g_game.options.animation_speed) {
        case (ANIMATION_SPEED_FAST):
            return 15;
        case (ANIMATION_SPEED_FULL):
            return 25;
        case (ANIMATION_SPEED_FASTER):
            return 8;
        case (ANIMATION_SPEED_NONE):
            return 1;
    }
    slog_fatal("Attack animation duration is invalid %i", g_game.options.animation_speed);
    return 0;
}

void queue_level_transition(LevelIndex next_level, TransitionType type, int32_t duration_in_frames) {
    if (in_level_transition()) return;
    g_game.level_transitions.start_frame = g_game.frame;
    g_game.level_transitions.duration = duration_in_frames;
    g_game.level_transitions.type = type;
    g_game.level_transitions.next_level = next_level;
    g_game.level_transitions.in_transition = true;
}

bool in_level_transition() {
    return g_game.level_transitions.in_transition;
}
