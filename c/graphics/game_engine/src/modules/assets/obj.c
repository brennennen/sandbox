
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/logger.h"
#include "modules/assets/obj.h"

#include "modules/assets/obj.h"
#include "modules/graphics/graphics_types.h"

static bool is_vertex_equal(const vertex_t* v1, const vertex_t* v2) {
    const float EPSILON = 0.00001f;
    if (fabsf(v1->pos[0] - v2->pos[0]) > EPSILON)
        return false;
    if (fabsf(v1->pos[1] - v2->pos[1]) > EPSILON)
        return false;
    if (fabsf(v1->pos[2] - v2->pos[2]) > EPSILON)
        return false;
    if (fabsf(v1->uv[0] - v2->uv[0]) > EPSILON)
        return false;
    if (fabsf(v1->uv[1] - v2->uv[1]) > EPSILON)
        return false;
    if (fabsf(v1->normal[0] - v2->normal[0]) > EPSILON)
        return false;
    if (fabsf(v1->normal[1] - v2->normal[1]) > EPSILON)
        return false;
    if (fabsf(v1->normal[2] - v2->normal[2]) > EPSILON)
        return false;

    return true;
}

bool load_obj(const char* filepath, mesh_data_t* out_mesh) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        log_error("Failed to open OBJ file: %s", filepath);
        return false;
    }

    vec3_t* temp_pos  = NULL;
    int     pos_count = 0;
    int     pos_cap   = 0;
    float*  temp_uvs  = NULL;
    int     uv_count  = 0;
    int     uv_cap    = 0;

    vec3_t* temp_norms = NULL;
    int     norm_count = 0;
    int     norm_cap   = 0;

    vertex_t* unique_verts = NULL;
    int       vert_count = 0, vert_cap = 0;
    uint16_t* indices   = NULL;
    int       ind_count = 0, ind_cap = 0;

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            if (pos_count >= pos_cap) {
                pos_cap  = pos_cap == 0 ? 128 : pos_cap * 2;
                temp_pos = realloc(temp_pos, pos_cap * sizeof(vec3_t));
            }
            sscanf(
                line,
                "v %f %f %f",
                &temp_pos[pos_count].x,
                &temp_pos[pos_count].y,
                &temp_pos[pos_count].z
            );
            pos_count++;
        } else if (strncmp(line, "vt ", 3) == 0) {
            if (uv_count * 2 >= uv_cap) {
                uv_cap   = uv_cap == 0 ? 256 : uv_cap * 2;
                temp_uvs = realloc(temp_uvs, uv_cap * sizeof(float));
            }
            sscanf(line, "vt %f %f", &temp_uvs[uv_count * 2], &temp_uvs[uv_count * 2 + 1]);
            uv_count++;
        } else if (strncmp(line, "f ", 2) == 0) {
            int p[3], t[3], n[3];
            int matches = sscanf(
                line,
                "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &p[0],
                &t[0],
                &n[0],
                &p[1],
                &t[1],
                &n[1],
                &p[2],
                &t[2],
                &n[2]
            );
            if (matches != 9)
                continue;

            if (ind_count + 3 > ind_cap) {
                ind_cap = ind_cap == 0 ? 128 : ind_cap * 2;
                indices = realloc(indices, ind_cap * sizeof(uint16_t));
            }

            for (int i = 0; i < 3; i++) {
                int pos_idx  = p[i] - 1;
                int uv_idx   = t[i] - 1;
                int norm_idx = n[i] - 1;

                vertex_t v;
                v.pos[0] = temp_pos[pos_idx].x;
                v.pos[1] = temp_pos[pos_idx].y;
                v.pos[2] = temp_pos[pos_idx].z;

                v.uv[0] = temp_uvs[uv_idx * 2];
                v.uv[1] = temp_uvs[uv_idx * 2 + 1];

                v.color = (vec4_t){1.0f, 1.0f, 1.0f, 1.0f};

                v.normal[0] = temp_norms[norm_idx].x;
                v.normal[1] = temp_norms[norm_idx].y;
                v.normal[2] = temp_norms[norm_idx].z;

                int found_index = -1;

                for (int j = 0; j < vert_count; j++) {
                    if (is_vertex_equal(&unique_verts[j], &v)) {
                        found_index = j;
                        break;
                    }
                }

                if (found_index != -1) {
                    indices[ind_count++] = (uint16_t)found_index;
                } else {
                    if (vert_count >= vert_cap) {
                        vert_cap     = vert_cap == 0 ? 128 : vert_cap * 2;
                        unique_verts = realloc(unique_verts, vert_cap * sizeof(vertex_t));
                    }
                    unique_verts[vert_count] = v;
                    indices[ind_count++]     = (uint16_t)vert_count;
                    vert_count++;
                }
            }
        } else if (strncmp(line, "vn ", 3) == 0) {
            if (norm_count >= norm_cap) {
                norm_cap   = norm_cap == 0 ? 128 : norm_cap * 2;
                temp_norms = realloc(temp_norms, norm_cap * sizeof(vec3_t));
            }
            sscanf(
                line,
                "vn %f %f %f",
                &temp_norms[norm_count].x,
                &temp_norms[norm_count].y,
                &temp_norms[norm_count].z
            );
            norm_count++;
        }
    }

    free(temp_pos);
    free(temp_uvs);
    free(temp_norms);
    fclose(file);

    out_mesh->vertices     = unique_verts;
    out_mesh->vertex_count = vert_count;
    out_mesh->indices      = indices;
    out_mesh->index_count  = ind_count;

    log_info("Loaded OBJ: %d Unique Vertices, %d Indices.", vert_count, ind_count);
    return true;
}

void free_mesh(mesh_data_t* mesh) {
    if (mesh->vertices) {
        free(mesh->vertices);
    }
    if (mesh->indices) {
        free(mesh->indices);
    }

    mesh->vertices     = NULL;
    mesh->indices      = NULL;
    mesh->vertex_count = 0;
    mesh->index_count  = 0;
}