

#include "engine/modules/debug_imgui/debug_imgui.h"

#include "engine/core/game_engine.h"

#include "engine/debug/engine_diagnostics.h"

#include "engine_diagnostics.h"

const char* format_number_commas(uint32_t number, char* out_buffer, uint8_t out_buffer_size) {
    char temp[32];
    int  len = snprintf(temp, sizeof(temp), "%u", number);

    int out_idx = 0;
    for (int i = 0; i < len; i++) {
        out_buffer[out_idx++] = temp[i];
        if ((len - i - 1) % 3 == 0 && i < len - 1) {
            out_buffer[out_idx++] = ',';
        }
    }
    out_buffer[out_idx] = '\0';

    return out_buffer;
}

void engine_diagnostics_draw_panel(game_engine_t* game_engine) {
    // igShowDemoWindow(NULL); // demo kitchen sink, useful for seeing capabilities
    igBegin("Engine Debug/Diagnostics", NULL, 0);

    igCheckbox("Show Debug Widgets (F3)", &game_engine->show_debug_widgets);

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

    if (igCollapsingHeader_TreeNodeFlags("Camera", ImGuiTreeNodeFlags_None)) {
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

        if (igCollapsingHeader_TreeNodeFlags("Render Stats", ImGuiTreeNodeFlags_DefaultOpen)) {

            uint32_t total_draw_calls =
                game_engine->last_frame_render_stats.forward_pass_draw_calls +
                game_engine->last_frame_render_stats.shadow_pass_draw_calls;
            char forward_pass_triangles_string[32];
            char shadow_pass_triangles_string[32];
            format_number_commas(
                game_engine->last_frame_render_stats.forward_pass_drawn_triangles,
                forward_pass_triangles_string,
                sizeof(forward_pass_triangles_string)
            );
            format_number_commas(
                game_engine->last_frame_render_stats.shadow_pass_drawn_triangles,
                shadow_pass_triangles_string,
                sizeof(shadow_pass_triangles_string)
            );

            if (igCollapsingHeader_TreeNodeFlags("Totals", ImGuiTreeNodeFlags_DefaultOpen)) {
                igText("Draw Calls: %u", total_draw_calls);
            }

            if (igCollapsingHeader_TreeNodeFlags("Forward Pass", ImGuiTreeNodeFlags_None)) {
                igText(
                    "Draw Calls: %u", game_engine->last_frame_render_stats.forward_pass_draw_calls
                );
                igText(
                    "Meshes Drawn: %u",
                    game_engine->last_frame_render_stats.forward_pass_drawn_meshes
                );
                igText(
                    "Meshes Culled: %u",
                    game_engine->main_scene.object_count -
                        game_engine->last_frame_render_stats.forward_pass_drawn_meshes
                );
                igText("Triangles: %s", forward_pass_triangles_string);
            }

            if (igCollapsingHeader_TreeNodeFlags("Shadow Pass", ImGuiTreeNodeFlags_None)) {
                igText(
                    "Draw Calls: %u", game_engine->last_frame_render_stats.shadow_pass_draw_calls
                );
                igText(
                    "Meshes Drawn: %u",
                    game_engine->last_frame_render_stats.shadow_pass_drawn_meshes
                );
                igText(
                    "Meshes Culled: %u",
                    game_engine->main_scene.object_count -
                        game_engine->last_frame_render_stats.shadow_pass_drawn_meshes
                );
                igText("Triangles: %s", shadow_pass_triangles_string);
            }

            if (igCollapsingHeader_TreeNodeFlags("Misc", ImGuiTreeNodeFlags_DefaultOpen)) {
                // TODO: skybox, post processing, etc.
            }

            igText("Memory (VRAM)");
            float vram_tex_mb = (float)game_engine->vram_texture_bytes / (1024.0f * 1024.0f);
            float vram_geo_mb = (float)game_engine->vram_geometry_bytes / (1024.0f * 1024.0f);
            igText("Textures: %.1f MB", vram_tex_mb);
            igText("Geometry: %.1f MB", vram_geo_mb);
            igText("Total:    %.1f MB", vram_tex_mb + vram_geo_mb);
        }

        igSeparator();
        igText("Texture Streaming Settings");
        int              previous_clamp      = game_engine->texture_mip_clamp;
        ImGuiSliderFlags mip_map_clamp_flags = 0;
        if (igSliderInt(
                "Mipmap Clamp",
                &game_engine->texture_mip_clamp,
                0,
                4,
                "Skip %d Mips",
                mip_map_clamp_flags
            )) {
            igTextColored((ImVec4){1.0f, 1.0f, 0.0f, 1.0f}, "Requires scene reload!");
            // TODO: hot reload
            // if (previous_clamp != g_texture_mip_clamp) {
            //     pak_loader_reload_scene(game_engine);
            // }
        }
        if (igIsItemHovered(ImGuiHoveredFlags_None)) {
            igSetTooltip("0 = 4K\n1 = 2K (-75%% VRAM)\n2 = 1K (-93%% VRAM)");
        }
    }

    if (igCollapsingHeader_TreeNodeFlags("Environment", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (igCollapsingHeader_TreeNodeFlags("Sun", ImGuiTreeNodeFlags_None)) {
            static float sun_azimuth        = 0.0f;
            static float sun_elevation      = 80.0f;
            static bool  angles_initialized = false;

            if (!angles_initialized) {
                vec3_t start_dir = game_engine->environment.sun_direction;
                sun_elevation    = asinf(-start_dir.z) * (180.0f / M_PI);
                sun_azimuth      = atan2f(start_dir.y, start_dir.x) * (180.0f / M_PI);
                if (sun_azimuth < 0.0f)
                    sun_azimuth += 360.0f;
                angles_initialized = true;
            }

            bool direction_changed = false;
            direction_changed |= igSliderFloat(
                "Azimuth", &sun_azimuth, 0.0f, 360.0f, "%.1f deg", 0
            );
            direction_changed |= igSliderFloat(
                "Elevation", &sun_elevation, 1.0f, 89.0f, "%.1f deg", 0
            );

            if (direction_changed) {
                float az_rad                             = sun_azimuth * (M_PI / 180.0f);
                float el_rad                             = sun_elevation * (M_PI / 180.0f);
                game_engine->environment.sun_direction.x = cosf(el_rad) * cosf(az_rad);
                game_engine->environment.sun_direction.y = cosf(el_rad) * sinf(az_rad);
                game_engine->environment.sun_direction.z = -sinf(el_rad);
            }

            igColorEdit3("Color", (float*)&game_engine->environment.sun_color.data, 0);
            igDragFloat(
                "Intensity", &game_engine->environment.sun_intensity, 0.1f, 0.0f, 0.0f, "%.2f", 0
            );
        }
        igSliderFloat(
            "Roughness Bias", &game_engine->environment.roughness_bias, -1.0f, 1.0f, "%.3f", 0
        );
        igSliderFloat(
            "Metallic Bias", &game_engine->environment.metallic_bias, -1.0f, 1.0f, "%.3f", 0
        );
    }

    igEnd();
}
