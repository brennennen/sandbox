

#include "engine/modules/assets/pak_loader.h"

#include "engine/core/logger.h"
#include "engine/core/vfs.h"
#include "engine/modules/assets/obj.h"
#include "engine/modules/graphics/graphics.h"
#include "pak_loader.h"

static void load_geometry_from_pak(game_engine_t* engine, world_pak_t* header, void* raw_pak_data) {
    log_info("Uploading raw PAK data to Vulkan...");

    pak_vertex_t* loaded_vertices = (pak_vertex_t*)((uint8_t*)raw_pak_data + header->vertex_offset);
    uint32_t*     loaded_indices  = (uint32_t*)((uint8_t*)raw_pak_data + header->index_offset);
    pak_mesh_t*   loaded_meshes   = (pak_mesh_t*)((uint8_t*)raw_pak_data + header->mesh_offset);
    pak_entity_t* loaded_entities = (pak_entity_t*)((uint8_t*)raw_pak_data + header->entity_offset);
    pak_texture_t* loaded_textures = (pak_texture_t*)((uint8_t*)raw_pak_data +
                                                      header->texture_offset);

    texture_handle_t gpu_textures[1024];
    for (uint32_t t = 0; t < header->texture_count; t++) {
        pak_texture_t* tex_def = &loaded_textures[t];
        image_t        img     = {
                       .width      = tex_def->width,
                       .height     = tex_def->height,
                       .channels   = tex_def->channels,
                       .size       = tex_def->byte_size,
                       .pixels     = (uint8_t*)raw_pak_data + tex_def->byte_offset,
                       .mip_levels = tex_def->mip_levels,
                       .is_cubemap = false
        };

        if (img.size == 0) {
            gpu_textures[t] = engine->default_tex;
            continue;
        }
        gpu_textures[t] = graphics_upload_texture(engine->graphics, &img, tex_def->format);
    }

    for (uint32_t i = 0; i < header->mesh_count; i++) {
        pak_mesh_t* mesh_def = &loaded_meshes[i];

        mesh_data_t raw_mesh_data = {
            .vertices        = (vertex_t*)&loaded_vertices[mesh_def->vertex_offset],
            .vertex_count    = mesh_def->vertex_count,
            .indices         = &loaded_indices[mesh_def->index_offset],
            .index_count     = mesh_def->index_count,
            .bounding_center = mesh_def->bounding_center,
            .bounding_radius = mesh_def->bounding_radius,
        };

        mesh_handle_t vram_handle = graphics_upload_mesh(engine->graphics, &raw_mesh_data);

        texture_handle_t mesh_tex = (mesh_def->base_color_texture_id >= 0)
                                        ? gpu_textures[mesh_def->base_color_texture_id]
                                        : engine->default_tex;

        texture_handle_t norm_tex = (mesh_def->normal_texture_id >= 0)
                                        ? gpu_textures[mesh_def->normal_texture_id]
                                        : engine->default_normal_tex;

        texture_handle_t ao_mr_tex = (mesh_def->ao_roughness_metallic_texture_id >= 0)
                                         ? gpu_textures[mesh_def->ao_roughness_metallic_texture_id]
                                         : engine->default_ao_metallic_roughness_tex;

        mat4_t final_transform = mat4_identity();
        for (uint32_t e = 0; e < header->entity_count; e++) {
            if (loaded_entities[e].model_id == mesh_def->model_id) {
                final_transform = loaded_entities[e].transform;
                break;
            }
        }

        uint32_t obj_idx                              = engine->main_scene.object_count++;
        engine->main_scene.objects[obj_idx].mesh      = vram_handle;
        engine->main_scene.objects[obj_idx].transform = final_transform;
        engine->main_scene.objects[obj_idx].material  = graphics_create_material(
            engine->graphics,
            mesh_tex,
            norm_tex,
            ao_mr_tex,
            mesh_def->is_alpha_masked,
            mesh_def->metallic_factor,
            mesh_def->roughness_factor
        );
    }
}

void load_pak_file(game_engine_t* game_engine, game_engine_init_config_t* engine_init_config) {
    void* raw_pak_data = vfs_get_mounted_archive_pointer(engine_init_config->initial_pak_path);
    if (raw_pak_data) {
        world_pak_t* header = (world_pak_t*)raw_pak_data;

        log_info(
            "sun: az: %0.2f, el: %0.2f, dir: %0.2f, %0.2f, %0.2f",
            header->environment.sun_azimuth,
            header->environment.sun_elevation,
            header->environment.sun_direction.x,
            header->environment.sun_direction.y,
            header->environment.sun_direction.z
        );
        game_engine->environment.sun_direction = header->environment.sun_direction;
        game_engine->environment.sun_color     = header->environment.sun_color;
        game_engine->environment.sun_intensity = header->environment.sun_intensity;

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
            game_engine->environment.skybox_texture = graphics_upload_texture(
                game_engine->graphics, &skybox_img, PAK_TEX_FORMAT_RGBA32F
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
                game_engine->graphics, &irr_img, PAK_TEX_FORMAT_RGBA32F
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
                game_engine->graphics, &pref_img, PAK_TEX_FORMAT_RGBA32F
            );

            // graphics_update_global_environment(
            //     game_engine->graphics,
            //     game_engine->environment.skybox_texture, irr_tex, pref_tex
            // );

            graphics_update_global_environment(
                game_engine->graphics,
                game_engine->environment.skybox_texture,
                game_engine->environment.skybox_texture, // irr_tex
                game_engine->environment.skybox_texture  // pref_tex
            );
            log_info("Successfully uploaded HDRI Skybox to GPU!");

            load_geometry_from_pak(game_engine, header, raw_pak_data);
        } else {
            game_engine->environment.skybox_texture = game_engine->default_tex; // Fallback
        }
    }
}

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
    log_info(
        "sun: az: %0.2f, el: %0.2f, ",
        header->environment.sun_azimuth,
        header->environment.sun_elevation
    );
    out_env->sun_color     = header->environment.sun_color;
    out_env->sun_intensity = header->environment.sun_intensity;

    out_env->ambient_tint = header->environment.ambient_tint;

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

    // load_geometry_from_pak(game_engine, header, raw_pak_data);
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
