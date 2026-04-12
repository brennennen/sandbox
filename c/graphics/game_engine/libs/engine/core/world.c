

#include <stdio.h>
#include <stdlib.h>

#include "engine/core/logger.h"
#include "engine/core/pak_format.h"
#include "engine/core/vfs.h"
#include "engine/core/world.h"
#include "engine/modules/assets/image.h"

pak_entity_t loaded_entities[MAX_LOADED_ENTITIES];
uint32_t     loaded_entity_count = 0;

pak_mesh_t loaded_meshes[MAX_LOADED_MESHES];
uint32_t   loaded_mesh_count = 0;

pak_vertex_t loaded_vertices[MAX_LOADED_VERTICES];
uint32_t     loaded_vertices_count = 0;

uint32_t loaded_indices[MAX_LOADED_INDICES];
uint32_t loaded_indices_count = 0;

pak_texture_t loaded_textures[MAX_LOADED_TEXTURES];
uint32_t      loaded_texture_count = 0;

bool world_load_chunk(uint32_t chunk_id) {
    uint64_t byte_offset;

    FILE* chunk_file = vfs_open_chunk(chunk_id, &byte_offset);
    if (!chunk_file) {
        log_error("World: Failed to open chunk %d from VFS", chunk_id);
        return false;
    }

    fseek(chunk_file, byte_offset, SEEK_SET);

    pak_chunk_header_t chunk;
    fread(&chunk, sizeof(pak_chunk_header_t), 1, chunk_file);

    if (chunk.entity_count > MAX_LOADED_ENTITIES || chunk.mesh_count > MAX_LOADED_MESHES ||
        chunk.vertex_count > MAX_LOADED_VERTICES || chunk.index_count > MAX_LOADED_INDICES ||
        chunk.texture_count > MAX_LOADED_TEXTURES) {

        log_error("World: Chunk data exceeds RAM limits!");
        return false;
    }

    fread(loaded_entities, sizeof(pak_entity_t), chunk.entity_count, chunk_file);
    loaded_entity_count = chunk.entity_count;

    fread(loaded_meshes, sizeof(pak_mesh_t), chunk.mesh_count, chunk_file);
    loaded_mesh_count = chunk.mesh_count;

    fread(loaded_vertices, sizeof(pak_vertex_t), chunk.vertex_count, chunk_file);
    loaded_vertices_count = chunk.vertex_count;

    fread(loaded_indices, sizeof(uint32_t), chunk.index_count, chunk_file);
    loaded_indices_count = chunk.index_count;

    fread(loaded_textures, sizeof(pak_texture_t), chunk.texture_count, chunk_file);
    loaded_texture_count = chunk.texture_count;

    log_info(
        "World: Streamed Chunk %d. (Verts: %d, Textures: %d)",
        chunk_id,
        loaded_vertices_count,
        loaded_texture_count
    );

    return true;
}

bool world_load_texture_image(uint32_t tex_idx, image_t* out_img) {
    if (tex_idx >= loaded_texture_count)
        return false;

    pak_texture_t* info = &loaded_textures[tex_idx];

    uint64_t junk_offset;
    FILE*    file = vfs_open_chunk(0, &junk_offset);
    if (!file) {
        return false;
    }

    fseek(file, info->byte_offset, SEEK_SET);

    out_img->width    = info->width;
    out_img->height   = info->height;
    out_img->channels = info->channels;

    if (info->format == PAK_TEX_FORMAT_RGBA8) {
        out_img->size   = info->byte_size;
        out_img->pixels = malloc(info->byte_size);
        fread(out_img->pixels, 1, info->byte_size, file);
        return true;
    } else if (info->format == PAK_TEX_FORMAT_PNG) {
        uint8_t* compressed_buffer = malloc(info->byte_size);
        fread(compressed_buffer, 1, info->byte_size, file);
        bool success = image_load_from_memory(compressed_buffer, info->byte_size, out_img);
        if (success) {
            out_img->size = out_img->width * out_img->height * 4;
        }
        free(compressed_buffer);
        return success;
    }

    return false;
}
