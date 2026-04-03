#include <stdlib.h>
#include <string.h>

#include "core/logger.h"
#include "core/math/mat4_math.h"
#include "core/scene.h"
#include "gltf.h"
#include "modules/assets/image.h"
#include "modules/graphics/graphics.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

static bool extract_primitive2(cgltf_primitive* primitive, mesh_data_t* out_mesh) {
    if (primitive->attributes_count == 0)
        return false;

    out_mesh->vertex_count = primitive->attributes[0].data->count;
    out_mesh->vertices     = calloc(out_mesh->vertex_count, sizeof(vertex_t));

    for (cgltf_size i = 0; i < primitive->attributes_count; i++) {
        cgltf_attribute* attribute = &primitive->attributes[i];

        if (attribute->type == cgltf_attribute_type_position) {
            for (cgltf_size v = 0; v < out_mesh->vertex_count; v++) {
                cgltf_accessor_read_float(attribute->data, v, out_mesh->vertices[v].pos, 3);
            }
        } else if (attribute->type == cgltf_attribute_type_normal) {
            for (cgltf_size v = 0; v < out_mesh->vertex_count; v++) {
                cgltf_accessor_read_float(attribute->data, v, out_mesh->vertices[v].normal, 3);
            }
        } else if (attribute->type == cgltf_attribute_type_texcoord) {
            for (cgltf_size v = 0; v < out_mesh->vertex_count; v++) {
                cgltf_accessor_read_float(attribute->data, v, out_mesh->vertices[v].uv, 2);
            }
        }
    }

    if (primitive->indices != NULL) {
        out_mesh->index_count = primitive->indices->count;
        out_mesh->indices     = malloc(out_mesh->index_count * sizeof(uint32_t));

        for (cgltf_size i = 0; i < out_mesh->index_count; i++) {
            uint32_t index_val   = cgltf_accessor_read_index(primitive->indices, i);
            out_mesh->indices[i] = index_val; //(uint16_t)index_val;
        }
    } else {
        out_mesh->index_count = 0;
        out_mesh->indices     = NULL;
    }

    return true;
}

static bool extract_primitive(cgltf_primitive* primitive, mesh_data_t* out_mesh) {
    if (primitive->attributes_count == 0)
        return false;

    out_mesh->vertex_count = primitive->attributes[0].data->count;
    out_mesh->vertices     = calloc(out_mesh->vertex_count, sizeof(vertex_t));
    vec4_t base_color      = {1.0f, 1.0f, 1.0f, 1.0f};

    if (primitive->material && primitive->material->has_pbr_metallic_roughness) {
        cgltf_float* factor = primitive->material->pbr_metallic_roughness.base_color_factor;
        base_color          = (vec4_t){factor[0], factor[1], factor[2], factor[3]};
    }

    for (cgltf_size v = 0; v < out_mesh->vertex_count; v++) {
        out_mesh->vertices[v].color = base_color;
    }

    for (cgltf_size i = 0; i < primitive->attributes_count; i++) {
        cgltf_attribute* attribute = &primitive->attributes[i];

        if (attribute->type == cgltf_attribute_type_position) {
            for (cgltf_size v = 0; v < out_mesh->vertex_count; v++) {
                cgltf_accessor_read_float(attribute->data, v, out_mesh->vertices[v].pos, 3);
            }
        } else if (attribute->type == cgltf_attribute_type_normal) {
            for (cgltf_size v = 0; v < out_mesh->vertex_count; v++) {
                cgltf_accessor_read_float(attribute->data, v, out_mesh->vertices[v].normal, 3);
            }
        }

        else if (attribute->type == cgltf_attribute_type_texcoord && attribute->index == 0) {
            for (cgltf_size v = 0; v < out_mesh->vertex_count; v++) {
                cgltf_accessor_read_float(attribute->data, v, out_mesh->vertices[v].uv, 2);
            }
        }

        else if (attribute->type == cgltf_attribute_type_color && attribute->index == 0) {
            for (cgltf_size v = 0; v < out_mesh->vertex_count; v++) {

                cgltf_accessor_read_float(
                    attribute->data, v, (float*)&out_mesh->vertices[v].color, 4
                );
            }
        }
    }

    if (primitive->indices != NULL) {
        out_mesh->index_count = primitive->indices->count;
        out_mesh->indices     = malloc(out_mesh->index_count * sizeof(uint32_t));

        for (cgltf_size i = 0; i < out_mesh->index_count; i++) {
            out_mesh->indices[i] = cgltf_accessor_read_index(primitive->indices, i);
        }
    } else {
        out_mesh->index_count = 0;
        out_mesh->indices     = NULL;
    }

    return true;
}

//
static mat4_t mat4_gltf_to_zup() {
    mat4_t m = mat4_identity();

    m.data[1][1] = 0.0f;
    m.data[1][2] = 1.0f;
    m.data[2][1] = -1.0f;
    m.data[2][2] = 0.0f;

    return m;
}

static void process_gltf_node(
    cgltf_node*       node,
    mat4_t            parent_transform,
    scene_t*          scene,
    graphics_t*       gfx,
    texture_handle_t* texture_cache,
    cgltf_data*       gltf_data,
    texture_handle_t  default_tex
) {
    mat4_t local_transform = mat4_identity();
    if (node->has_matrix || node->has_translation || node->has_rotation || node->has_scale) {
        cgltf_float matrix[16];
        cgltf_node_transform_local(node, matrix);
        memcpy(&local_transform, matrix, sizeof(mat4_t));
    }

    mat4_t global_transform = mat4_mul(local_transform, parent_transform);

    if (node->mesh != NULL) {
        for (cgltf_size i = 0; i < node->mesh->primitives_count; i++) {
            mesh_data_t mesh_data;
            if (extract_primitive(&node->mesh->primitives[i], &mesh_data)) {
                mesh_handle_t handle = graphics_upload_mesh(gfx, &mesh_data);
                free(mesh_data.vertices);
                if (mesh_data.indices)
                    free(mesh_data.indices);

                texture_handle_t mat_tex = default_tex;

                cgltf_material* mat = node->mesh->primitives[i].material;
                if (mat && mat->has_pbr_metallic_roughness) {
                    cgltf_texture* tex_ptr = mat->pbr_metallic_roughness.base_color_texture.texture;

                    if (tex_ptr && texture_cache) {
                        cgltf_size tex_index = tex_ptr - gltf_data->textures;
                        mat_tex              = texture_cache[tex_index];
                    }
                }

                if (scene->object_count < MAX_SCENE_OBJECTS) {
                    scene->objects[scene->object_count++] = (render_object_t){
                        .mesh = handle, .texture = mat_tex, .transform = global_transform
                    };
                }
            }
        }
    }

    for (cgltf_size i = 0; i < node->children_count; i++) {
        process_gltf_node(
            node->children[i], global_transform, scene, gfx, texture_cache, gltf_data, default_tex
        );
    }
}

bool load_gltf_scene(
    const char*      filepath,
    scene_t*         out_scene,
    graphics_t*      gfx,
    texture_handle_t default_tex
) {
    cgltf_options options = {0};
    cgltf_data*   data    = NULL;

    if (cgltf_parse_file(&options, filepath, &data) != cgltf_result_success) {
        log_error("cgltf: Failed to parse %s", filepath);
        return false;
    }

    if (cgltf_load_buffers(&options, data, filepath) != cgltf_result_success) {
        log_error("cgltf: Failed to load buffers for %s", filepath);
        cgltf_free(data);
        return false;
    }

    cgltf_scene* gltf_scene = data->scene;
    if (!gltf_scene) {
        log_error("cgltf: No default scene found in %s", filepath);
        cgltf_free(data);
        return false;
    }

    texture_handle_t* texture_cache = NULL;
    if (data->textures_count > 0) {
        texture_cache = malloc(sizeof(texture_handle_t) * data->textures_count);

        for (cgltf_size i = 0; i < data->textures_count; i++) {
            cgltf_texture* gltf_tex = &data->textures[i];
            cgltf_image*   gltf_img = gltf_tex->image;
            texture_cache[i]        = default_tex;
            if (gltf_img && gltf_img->buffer_view) {
                void* tex_data = (uint8_t*)gltf_img->buffer_view->buffer->data +
                                 gltf_img->buffer_view->offset;
                int tex_size = (int)gltf_img->buffer_view->size;

                image_t img = {0};
                if (image_load_from_memory(tex_data, tex_size, &img)) {
                    img.size = img.width * img.height * 4;

                    texture_handle_t uploaded = graphics_upload_texture(gfx, &img);
                    if (uploaded.id != UINT32_MAX) {
                        texture_cache[i] = uploaded;
                    }
                    image_free(&img);
                }
            }
        }
    }

    out_scene->object_count = 0;
    mat4_t root_transform   = mat4_gltf_to_zup();

    for (cgltf_size i = 0; i < gltf_scene->nodes_count; i++) {
        process_gltf_node(
            gltf_scene->nodes[i], root_transform, out_scene, gfx, texture_cache, data, default_tex
        );
    }

    log_info(
        "cgltf: Successfully loaded scene %s with %u objects", filepath, out_scene->object_count
    );

    cgltf_free(data);
    return true;
}
