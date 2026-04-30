#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "engine/core/logger.h"
#include "engine/core/math/mat4_math.h"
#include "engine/core/scene.h"
#include "engine/modules/assets/image.h"
#include "engine/modules/graphics/graphics.h"

#include "gltf.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

static void calculate_tangents(mesh_data_t* mesh) {
    for (uint32_t i = 0; i < mesh->vertex_count; i++) {
        mesh->vertices[i].tangent = (vec4_t){0.0f, 0.0f, 0.0f, 0.0f};
    }

    vec3_t* bitangents = calloc(mesh->vertex_count, sizeof(vec3_t));

    uint32_t triangle_count = mesh->index_count > 0 ? mesh->index_count / 3
                                                    : mesh->vertex_count / 3;

    for (uint32_t t = 0; t < triangle_count; t++) {
        uint32_t i0 = mesh->index_count > 0 ? mesh->indices[t * 3 + 0] : t * 3 + 0;
        uint32_t i1 = mesh->index_count > 0 ? mesh->indices[t * 3 + 1] : t * 3 + 1;
        uint32_t i2 = mesh->index_count > 0 ? mesh->indices[t * 3 + 2] : t * 3 + 2;

        vertex_t* v0 = &mesh->vertices[i0];
        vertex_t* v1 = &mesh->vertices[i1];
        vertex_t* v2 = &mesh->vertices[i2];

        float e1x = v1->pos.data[0] - v0->pos.data[0];
        float e1y = v1->pos.data[1] - v0->pos.data[1];
        float e1z = v1->pos.data[2] - v0->pos.data[2];

        float e2x = v2->pos.data[0] - v0->pos.data[0];
        float e2y = v2->pos.data[1] - v0->pos.data[1];
        float e2z = v2->pos.data[2] - v0->pos.data[2];

        float duv1x = v1->uv.data[0] - v0->uv.data[0];
        float duv1y = v1->uv.data[1] - v0->uv.data[1];

        float duv2x = v2->uv.data[0] - v0->uv.data[0];
        float duv2y = v2->uv.data[1] - v0->uv.data[1];

        float det = duv1x * duv2y - duv2x * duv1y;
        float f   = 1.0f;
        if (det > 0.000001f || det < -0.000001f) {
            f = 1.0f / det;
        }

        if (f > 999999.0f || f < -999999.0f)
            f = 1.0f;

        float tx = f * (duv2y * e1x - duv1y * e2x);
        float ty = f * (duv2y * e1y - duv1y * e2y);
        float tz = f * (duv2y * e1z - duv1y * e2z);

        float bx = f * (-duv2x * e1x + duv1x * e2x);
        float by = f * (-duv2x * e1y + duv1x * e2y);
        float bz = f * (-duv2x * e1z + duv1x * e2z);

        v0->tangent.data[0] += tx;
        v0->tangent.data[1] += ty;
        v0->tangent.data[2] += tz;
        v1->tangent.data[0] += tx;
        v1->tangent.data[1] += ty;
        v1->tangent.data[2] += tz;
        v2->tangent.data[0] += tx;
        v2->tangent.data[1] += ty;
        v2->tangent.data[2] += tz;

        bitangents[i0].data[0] += bx;
        bitangents[i0].data[1] += by;
        bitangents[i0].data[2] += bz;
        bitangents[i1].data[0] += bx;
        bitangents[i1].data[1] += by;
        bitangents[i1].data[2] += bz;
        bitangents[i2].data[0] += bx;
        bitangents[i2].data[1] += by;
        bitangents[i2].data[2] += bz;
    }

    for (uint32_t i = 0; i < mesh->vertex_count; i++) {
        vec3_t n = mesh->vertices[i].normal;
        vec4_t t = mesh->vertices[i].tangent;
        vec3_t b = bitangents[i];

        // Gram-Schmidt process: t = t - n * dot(n, t)
        float dot_nt = n.data[0] * t.data[0] + n.data[1] * t.data[1] + n.data[2] * t.data[2];

        float ox = t.data[0] - n.data[0] * dot_nt;
        float oy = t.data[1] - n.data[1] * dot_nt;
        float oz = t.data[2] - n.data[2] * dot_nt;

        // Normalize
        float len = sqrtf(ox * ox + oy * oy + oz * oz);
        if (len > 0.0001f) {
            ox /= len;
            oy /= len;
            oz /= len;
        } else {
            ox = 1.0f;
            oy = 0.0f;
            oz = 0.0f;
        }

        // Calculate Handedness (W component) using dot(cross(N, T), B)
        float cx = n.data[1] * oz - n.data[2] * oy;
        float cy = n.data[2] * ox - n.data[0] * oz;
        float cz = n.data[0] * oy - n.data[1] * ox;

        float handedness = (cx * b.data[0] + cy * b.data[1] + cz * b.data[2]) < 0.0f ? -1.0f : 1.0f;

        mesh->vertices[i].tangent = (vec4_t){ox, oy, oz, handedness};
    }

    free(bitangents);
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
                cgltf_accessor_read_float(attribute->data, v, out_mesh->vertices[v].pos.data, 3);
            }
        } else if (attribute->type == cgltf_attribute_type_normal) {
            for (cgltf_size v = 0; v < out_mesh->vertex_count; v++) {
                cgltf_accessor_read_float(attribute->data, v, out_mesh->vertices[v].normal.data, 3);
            }
        } else if (attribute->type == cgltf_attribute_type_tangent) {
            for (cgltf_size v = 0; v < out_mesh->vertex_count; v++) {
                cgltf_accessor_read_float(
                    attribute->data, v, out_mesh->vertices[v].tangent.data, 4
                );
            }
        }

        else if (attribute->type == cgltf_attribute_type_texcoord && attribute->index == 0) {
            for (cgltf_size v = 0; v < out_mesh->vertex_count; v++) {
                cgltf_accessor_read_float(attribute->data, v, out_mesh->vertices[v].uv.data, 2);
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

                bool has_tangents = false;
                for (cgltf_size a = 0; a < node->mesh->primitives[i].attributes_count; a++) {
                    if (node->mesh->primitives[i].attributes[a].type ==
                        cgltf_attribute_type_tangent) {
                        has_tangents = true;
                        break;
                    }
                }
                if (!has_tangents) {
                    calculate_tangents(&mesh_data);
                }

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
                    material_handle_t mat_handle = graphics_create_material(
                        gfx, mat_tex, default_tex
                    );

                    scene->objects[scene->object_count++] = (render_object_t){
                        .mesh      = handle,
                        .material  = mat_handle,
                        .transform = global_transform
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

                    texture_handle_t uploaded = graphics_upload_texture(
                        gfx, &img, PAK_TEX_FORMAT_RGBA8_SRGB
                    );
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
