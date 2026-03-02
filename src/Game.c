#include <stddef.h>
#include <stdlib.h>
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

    menu_begin();
    return nullptr;
}

void *update(void *ptr) {
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

    // Draw from the backbuffer to the screen
    const float window_width = oct_WindowWidth();
    const float window_height = oct_WindowHeight();
    oct_SetDrawTarget(OCT_TARGET_SWAPCHAIN);
    oct_SetTextureCamerasEnabled(false);
    oct_LockCameras(g_game.render_camera);
    oct_DrawTextureExt(
            g_game.backbuffer,
            (Oct_Vec2){0, 0},
            (Oct_Vec2){window_width / VIRTUAL_WIDTH, window_height / VIRTUAL_HEIGHT},
            0, (Oct_Vec2){0, 0});

    return nullptr;
}

void shutdown(void *ptr) {
    oct_FreeAssetBundle(g_game.assets);
    oct_FreeAllocator(g_game.allocator);
}
