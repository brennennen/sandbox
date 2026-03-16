#include "obj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "modules/assets/obj.h"
#include "modules/graphics/graphics_types.h"

bool load_obj(const char* filepath, mesh_data_t* out_mesh) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("Failed to open OBJ file: %s\n", filepath);
        return false;
    }

    // Temporary arrays for unique positions and UVs
    vec3_t* temp_pos  = NULL;
    int     pos_count = 0, pos_cap = 0;
    float*  temp_uvs = NULL;
    int     uv_count = 0, uv_cap = 0; // Store as pairs [u, v, u, v...]

    // Final unrolled vertices
    vertex_t* final_verts = NULL;
    int       vert_count = 0, vert_cap = 0;

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        // Parse Vertex Positions
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
        }
        // Parse Texture Coordinates (UVs)
        else if (strncmp(line, "vt ", 3) == 0) {
            if (uv_count * 2 >= uv_cap) {
                uv_cap   = uv_cap == 0 ? 256 : uv_cap * 2;
                temp_uvs = realloc(temp_uvs, uv_cap * sizeof(float));
            }
            sscanf(line, "vt %f %f", &temp_uvs[uv_count * 2], &temp_uvs[uv_count * 2 + 1]);
            uv_count++;
        }
        // Parse Faces (Triangles)
        else if (strncmp(line, "f ", 2) == 0) {
            // We expect the format v/vt/vn v/vt/vn v/vt/vn
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

            if (matches != 9) {
                printf("OBJ Parse Error: Face format must be v/vt/vn and triangulated.\n");
                fclose(file);
                return false;
            }

            // Ensure we have enough space for 3 new vertices
            if (vert_count + 3 > vert_cap) {
                vert_cap    = vert_cap == 0 ? 128 : vert_cap * 2;
                final_verts = realloc(final_verts, vert_cap * sizeof(vertex_t));
            }

            // Unroll the face into 3 flat vertices
            for (int i = 0; i < 3; i++) {
                // OBJ indices start at 1, so we must subtract 1 for C arrays!
                int pos_idx = p[i] - 1;
                int uv_idx  = t[i] - 1;

                vertex_t v;
                v.pos[0] = temp_pos[pos_idx].x;
                v.pos[1] = temp_pos[pos_idx].y;
                v.pos[2] = temp_pos[pos_idx].z;

                // Default color to white so textures show up normally
                v.color = (vec4_t){1.0f, 1.0f, 1.0f, 1.0f};

                v.uv[0] = temp_uvs[uv_idx * 2];
                v.uv[1] = temp_uvs[uv_idx * 2 + 1];

                final_verts[vert_count++] = v;
            }
        }
    }

    // Clean up temporary arrays
    free(temp_pos);
    free(temp_uvs);
    fclose(file);

    out_mesh->vertices     = final_verts;
    out_mesh->vertex_count = vert_count;

    printf("Loaded OBJ: %d vertices.\n", vert_count);
    return true;
}

void free_mesh(mesh_data_t* mesh) {
    if (mesh->vertices) {
        free(mesh->vertices);
        mesh->vertices = NULL;
    }
    mesh->vertex_count = 0;
}
