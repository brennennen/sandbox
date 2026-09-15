

#include "engine/modules/assets/pak_loader.h"

#include "engine/core/logger.h"
#include "engine/core/vfs.h"
#include "engine/modules/assets/obj.h"
#include "engine/modules/graphics/graphics.h"
#include "pak_loader.h"

bool pak_loader_load_world(
    graphics_t*    graphics,
    scene_t*       out_scene,
    environment_t* out_env,
    const char*    pak_path
) {
    void* raw_pak_data = vfs_get_mounted_archive_pointer(pak_path);
    if (!raw_pak_data)
        return false;

    world_pak_t* header = (world_pak_t*)raw_pak_data;

    out_env->sun_direction = header->environment.sun_direction;
    out_env->sun_color     = header->environment.sun_color;
    out_env->sun_intensity = header->environment.sun_intensity;

    texture_pak_t* skybox_def = &header->environment.skybox_cubemap;
    if (skybox_def->data_size > 0) {
        void*   pixel_data = (uint8_t*)raw_pak_data + skybox_def->data_offset;
        image_t skybox_img = {
            .width      = skybox_def->width,
            .height     = skybox_def->height,
            .channels   = skybox_def->channels,
            .size       = skybox_def->data_size,
            .pixels     = pixel_data,
            .is_cubemap = true,
        };
        out_env->skybox_texture = graphics_upload_texture(
            graphics, &skybox_img, PAK_TEX_FORMAT_RGBA32F
        );

        texture_pak_t* irr_def = &header->environment.irradiance_map;
        image_t        irr_img = {
                   .width      = irr_def->width,
                   .height     = irr_def->height,
                   .channels   = irr_def->channels,
                   .size       = irr_def->data_size,
                   .pixels     = (uint8_t*)raw_pak_data + irr_def->data_offset,
                   .is_cubemap = true,
        };
        texture_handle_t irr_tex = graphics_upload_texture(
            graphics, &irr_img, PAK_TEX_FORMAT_RGBA32F
        );

        texture_pak_t* pref_def = &header->environment.prefiltered_env_map;
        image_t        pref_img = {
                   .width      = pref_def->width,
                   .height     = pref_def->height,
                   .channels   = pref_def->channels,
                   .size       = pref_def->data_size,
                   .pixels     = (uint8_t*)raw_pak_data + pref_def->data_offset,
                   .mip_levels = 5,
                   .is_cubemap = true,
        };
        texture_handle_t pref_tex = graphics_upload_texture(
            graphics, &pref_img, PAK_TEX_FORMAT_RGBA32F
        );

        // graphics_update_global_environment(
        //     game_engine->graphics,
        //     game_engine->environment.skybox_texture, irr_tex, pref_tex
        // );

        graphics_update_global_environment(
            graphics,
            out_env->skybox_texture,
            out_env->skybox_texture, // irr_tex
            out_env->skybox_texture  // pref_tex
        );
        log_info("Successfully uploaded HDRI Skybox to GPU!");
    }

    //load_geometry_from_pak(game_engine, header, raw_pak_data);
    log_info("Uploading raw PAK data to Vulkan...");
    pak_vertex_t* loaded_vertices = (pak_vertex_t*)((uint8_t*)raw_pak_data + header->vertex_offset);

    return true;
}

bool pak_loader_load_scene(graphics_t* graphics, scene_t* scene, const char* pak_path) {
    return false;
}

texture_handle_t pak_loader_load_skybox(graphics_t* graphics, const char* pak_path) {
    void* raw_pak_data = vfs_get_mounted_archive_pointer(pak_path);
    if (!raw_pak_data) {
        return (texture_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    world_pak_t*   header     = (world_pak_t*)raw_pak_data;
    texture_pak_t* skybox_def = &header->environment.skybox_cubemap;

    if (skybox_def->data_size == 0) {
        return (texture_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    image_t skybox_img = {
        .width      = skybox_def->width,
        .height     = skybox_def->height,
        .channels   = skybox_def->channels,
        .size       = skybox_def->data_size,
        .pixels     = (uint8_t*)raw_pak_data + skybox_def->data_offset,
        .is_cubemap = true,
    };
    texture_handle_t skybox_tex = graphics_upload_texture(
        graphics, &skybox_img, PAK_TEX_FORMAT_RGBA32F
    );

    texture_pak_t* irr_def = &header->environment.irradiance_map;
    image_t        irr_img = {
               .width      = irr_def->width,
               .height     = irr_def->height,
               .channels   = irr_def->channels,
               .size       = irr_def->data_size,
               .pixels     = (uint8_t*)raw_pak_data + irr_def->data_offset,
               .is_cubemap = true,
    };
    texture_handle_t irr_tex = graphics_upload_texture(graphics, &irr_img, PAK_TEX_FORMAT_RGBA32F);

    texture_pak_t* pref_def = &header->environment.prefiltered_env_map;
    image_t        pref_img = {
               .width      = pref_def->width,
               .height     = pref_def->height,
               .channels   = pref_def->channels,
               .size       = pref_def->data_size,
               .pixels     = (uint8_t*)raw_pak_data + pref_def->data_offset,
               .mip_levels = 5,
               .is_cubemap = true,
    };
    texture_handle_t pref_tex = graphics_upload_texture(
        graphics, &pref_img, PAK_TEX_FORMAT_RGBA32F
    );

    // TODO: fix irradiance and prefiltered map
    graphics_update_global_environment(graphics, skybox_tex, skybox_tex, skybox_tex);
    log_info("loaded skybox");

    return skybox_tex;
}
