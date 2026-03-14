#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include "Game.h"
#include "Structs.h"

Game g_game;

void *startup() {
    oct_Log("Starting the game.");

    // Octarine things
    g_game.backbuffer = oct_CreateSurface((Oct_Vec2){VIRTUAL_WIDTH, VIRTUAL_HEIGHT});
    g_game.world_camera = oct_CreateCamera();
    g_game.ui_camera = oct_CreateCamera();
    g_game.render_camera = oct_CreateCamera();
    g_game.assets = oct_LoadAssetBundle("data");
    g_game.allocator = oct_CreateHeapAllocator();

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

    menu_begin();
    return nullptr;
}

void *update(void *ptr) {
#ifndef NDEBUG
    static bool debug = true;
#else
    static bool debug = false;
#endif
    oct_SetDrawTarget(g_game.backbuffer);
    oct_DrawClear(&(Oct_Colour){.r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f});
    oct_SetTextureCamerasEnabled(true);

    if (g_game.level_index == LEVEL_INDEX_MENU) {
        const LevelIndex new_index = menu_update();
        if (new_index == LEVEL_INDEX_QUIT) {
            oct_Log("Quitting from the menu.");
            menu_end();
            abort();
        } else if (new_index != LEVEL_INDEX_MENU) {
            oct_Log("Switching from the menu to level index %i.", new_index);
            menu_end();
            g_game.level_index = new_index;
            level_begin();
        }
    } else {
        const LevelIndex new_index = level_update();
        if (new_index == LEVEL_INDEX_MENU) {
            oct_Log("Quitting from level %i to the menu.", g_game.level_index);
            level_end();
            g_game.level_index = new_index;
            menu_begin();
        } else if (new_index == LEVEL_INDEX_QUIT) {
            oct_Log("Quitting from level %i.", g_game.level_index);
            level_end();
            abort();
        } else if (new_index != g_game.level_index) {
            oct_Log("Going from level %i to level %i.", g_game.level_index, new_index);
            level_end();
            g_game.level_index = new_index;
            level_begin();
        }
    }


    // Toggle fullscreen
    if (oct_KeyDown(OCT_KEY_LALT) && oct_KeyPressed(OCT_KEY_RETURN))
        oct_SetFullscreen(!oct_WindowIsFullscreen());
    if (oct_KeyPressed(OCT_KEY_1))
        g_game.scale_mode = SCALE_MODE_INTEGER;
    if (oct_KeyPressed(OCT_KEY_2))
        g_game.scale_mode = SCALE_MODE_ASPECT_RATIO;
    if (oct_KeyPressed(OCT_KEY_3))
        g_game.scale_mode = SCALE_MODE_STRETCH;
    if (oct_KeyPressed(OCT_KEY_F1))
        debug = !debug;
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
    if (g_game.scale_mode == SCALE_MODE_INTEGER) {
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
    } else if (g_game.scale_mode == SCALE_MODE_ASPECT_RATIO) {
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

    if (debug) {
        // Draw debug info
        oct_DrawText(oct_GetAsset(g_game.assets, "fnt_small"),
                     (Oct_Vec2) {1, 0}, 1,
                     "Player: %i, %i\nState: %s\nAttack animation: %s",
                     g_game.player.pos[0], g_game.player.pos[1],
                     GAME_STATES[g_game.current_level.state],
                     level_in_attack_animation() ? "yes" : "no");
        oct_DrawText(oct_GetAsset(g_game.assets, "fnt_small"),
                     (Oct_Vec2) {500, 0}, 1,
                     "Logic: %.2f Refresh: %.2f\nPopup stack pointer: %i",
                     oct_GetLogicHz(),
                     oct_GetRenderFPS(),
                     g_game.current_level.popup_stack_pointer);
    }

    g_game.frame++;
    return nullptr;
}

void shutdown(void *ptr) {
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
