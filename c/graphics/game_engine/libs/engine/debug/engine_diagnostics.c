

#include "engine/modules/debug_imgui/debug_imgui.h"

#include "engine/core/game_engine.h"

#include "engine/debug/engine_diagnostics.h"

#include "engine_diagnostics.h"

void engine_diagnostics_draw_panel(game_engine_t* game_engine) {
    // igShowDemoWindow(NULL); // demo kitchen sink, useful for seeing capabilities
    igBegin("Engine Debug/Diagnostics", NULL, 0);

    if (igCollapsingHeader_TreeNodeFlags("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
        igText("is_paused: %d", game_engine->is_paused);
        igText("delta_time: %0.2f ms", game_engine->delta_time * 1000.0f);
        igText("fps (1s avg): %d", game_engine->fps_last_1s_avg);

        ImGuiIO* io = igGetIO_Nil();
        igText("Smoothed FPS: %.1f", io->Framerate);
        igText("Frame Time: %.3f ms", 1000.0f / io->Framerate);
    }

    if (igCollapsingHeader_TreeNodeFlags("Engine State", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool is_paused = game_engine->is_paused;
        if (igCheckbox("Paused (Esc)", &is_paused)) {
            game_engine->is_paused = is_paused;
            platform_set_relative_mouse(game_engine->platform, !is_paused);
        }
        int w, h;
        platform_get_window_size(game_engine->platform, &w, &h);
        igText("Resolution: %d x %d", w, h);
        igText("Active Scene Objects: %u", game_engine->main_scene.object_count);
    }

    if (igCollapsingHeader_TreeNodeFlags("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (game_engine->main_camera) {
            igDragFloat3(
                "Position", (float*)&game_engine->main_camera->pos, 0.1f, 0.0f, 0.0f, "%.2f", 0
            );
            igText("Yaw: %.2f", game_engine->main_camera->yaw);
            igText("Pitch: %.2f", game_engine->main_camera->pitch);
            float fov_deg = game_engine->main_camera->fov * (180.0f / M_PI);
            if (igSliderFloat("FOV (Degrees)", &fov_deg, 10.0f, 150.0f, "%.1f", 0)) {
                game_engine->main_camera->fov = fov_deg * (M_PI / 180.0f);
            }
            igDragFloat(
                "Near Plane", &game_engine->main_camera->near_plane, 0.01f, 0.001f, 10.0f, "%.3f", 0
            );
            igDragFloat(
                "Far Plane", &game_engine->main_camera->far_plane, 10.0f, 10.0f, 20000.0f, "%.1f", 0
            );
        }
    }

    if (igCollapsingHeader_TreeNodeFlags("Graphics", ImGuiTreeNodeFlags_DefaultOpen)) {
        igText("Draw Mode (F5): %s", draw_mode_names[game_engine->draw_mode]);

        bool is_frozen = game_engine->debug_freeze_culling;
        if (igCheckbox("Freeze Frustum Culling (F6)", &is_frozen)) {
            game_engine->debug_freeze_culling = is_frozen;
            if (is_frozen) {
                mat4_t inv_culling = mat4_inverse(game_engine->culling_view_proj);
                graphics_update_debug_frustum(game_engine->graphics, inv_culling);
            }
        }
    }

    if (igCollapsingHeader_TreeNodeFlags("Environment", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (igCollapsingHeader_TreeNodeFlags("Sun", ImGuiTreeNodeFlags_DefaultOpen)) {
            igDragFloat3(
                "Direction",
                (float*)&game_engine->environment.sun_direction.data,
                0.1f,
                0.0f,
                0.0f,
                "%.2f",
                0
            );
            igColorEdit3("Color", (float*)&game_engine->environment.sun_color.data, 0);
            igDragFloat(
                "Intensity", &game_engine->environment.sun_intensity, 0.1f, 0.0f, 0.0f, "%.2f", 0
            );
        }
    }

    igEnd();
}
