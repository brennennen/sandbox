
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "bc7/bc7enc.h"
#include "engine/core/logger.h"
#include "engine/core/math/mat4.h"
#include "gltf_baker.h"
#include "mesh_utilities.h"
#include "shared/scene_types.h"


/**
 * Calculates the tangent and bitangent vectors for a single triangle and adds
 * them to its vertices.
 *
 * A raw 3D mesh has no idea how a 2D image wraps around it. When a lighting
 * shader uses a normal map, it needs a "bridge" to translate the 2D pixel directions
 * into 3D world directions.  Because a vertex might be shared by multiple triangles,
 * we just "accumulate" (add) the vectors together here and orthogonalize them later.
 *
 * @see https://terathon.com/blog/tangent-space.html
 *
 * @param vertex0 The first vertex of the triangle.
 * @param vertex1 The second vertex of the triangle.
 * @param vertex2 The third vertex of the triangle.
 * @param bitangent0 Pointer to the accumulated bitangent tracker for vertex 0.
 * @param bitangent1 Pointer to the accumulated bitangent tracker for vertex 1.
 * @param bitangent2 Pointer to the accumulated bitangent tracker for vertex 2.
 */
void accumulate_triangle_tangents(
    pak_vertex_t* vertex0,
    pak_vertex_t* vertex1,
    pak_vertex_t* vertex2,
    vec3_t*       bitangent0,
    vec3_t*       bitangent1,
    vec3_t*       bitangent2
) {
    // Calculate the edges of the triangle
    float edge1_x = vertex1->pos.x - vertex0->pos.x;
    float edge1_y = vertex1->pos.y - vertex0->pos.y;
    float edge1_z = vertex1->pos.z - vertex0->pos.z;

    float edge2_x = vertex2->pos.x - vertex0->pos.x;
    float edge2_y = vertex2->pos.y - vertex0->pos.y;
    float edge2_z = vertex2->pos.z - vertex0->pos.z;

    // Calculate the 2D texture coordinate (UV) edges of the triangle
    float delta_uv1_u = vertex1->uv.u - vertex0->uv.u;
    float delta_uv1_v = vertex1->uv.v - vertex0->uv.v;

    float delta_uv2_u = vertex2->uv.u - vertex0->uv.u;
    float delta_uv2_v = vertex2->uv.v - vertex0->uv.v;

    // Calculate the determinant of the UV matrix to scale the vectors appropriately
    float determinant         = (delta_uv1_u * delta_uv2_v - delta_uv2_u * delta_uv1_v);
    float inverse_determinant = (determinant == 0.0f) ? 1.0f : 1.0f / determinant;

    // Solve the linear system for the 3D Tangent (Texture U axis) and Bitangent (Texture V axis)
    float tangent_x = inverse_determinant * (delta_uv2_v * edge1_x - delta_uv1_v * edge2_x);
    float tangent_y = inverse_determinant * (delta_uv2_v * edge1_y - delta_uv1_v * edge2_y);
    float tangent_z = inverse_determinant * (delta_uv2_v * edge1_z - delta_uv1_v * edge2_z);

    float bitangent_x = inverse_determinant * (-delta_uv2_u * edge1_x + delta_uv1_u * edge2_x);
    float bitangent_y = inverse_determinant * (-delta_uv2_u * edge1_y + delta_uv1_u * edge2_y);
    float bitangent_z = inverse_determinant * (-delta_uv2_u * edge1_z + delta_uv1_u * edge2_z);

    // Accumulate the results into the running totals for each vertex
    vertex0->tangent.x += tangent_x;
    vertex0->tangent.y += tangent_y;
    vertex0->tangent.z += tangent_z;

    vertex1->tangent.x += tangent_x;
    vertex1->tangent.y += tangent_y;
    vertex1->tangent.z += tangent_z;

    vertex2->tangent.x += tangent_x;
    vertex2->tangent.y += tangent_y;
    vertex2->tangent.z += tangent_z;

    bitangent0->x += bitangent_x;
    bitangent0->y += bitangent_y;
    bitangent0->z += bitangent_z;

    bitangent1->x += bitangent_x;
    bitangent1->y += bitangent_y;
    bitangent1->z += bitangent_z;

    bitangent2->x += bitangent_x;
    bitangent2->y += bitangent_y;
    bitangent2->z += bitangent_z;
}

/**
 * Cleans up a vertex's tangent vector to make it perfectly 90 degrees to the normal.
 *
 * When a vertex is shared by multiple triangles, its accumulated tangent becomes skewed.
 * This function "straightens" out this skewing problem. It also checks if the artist flipped
 * the texture (mirrored UVs) and saves that state as a 1 or -1 in the 4th slot. Skips sending
 * an entire third vector to the GPU.
 *
 * @see "Introduction to 3D Game Programming With DirectX 11" Chapter 18 Normal Mapping and
 * Displacement Mapping
 *
 * @param vertex Pointer to the vertex containing the normal and the messy tangent.
 * @param accumulated_bitangent Used strictly to check if the texture was mirrored.
 */
void orthogonalize_vertex_tangent(pak_vertex_t* vertex, vec3_t* accumulated_bitangent) {
    vec3_t accumulated_tangent = {vertex->tangent.x, vertex->tangent.y, vertex->tangent.z};

    // T' = normalize(T - N * dot(N, T))
    vec3_t orthogonal_tangent = vec3_gram_schmidt(vertex->normal, accumulated_tangent);

    // B = N * T'
    float perfect_bitangent_x = vertex->normal.y * orthogonal_tangent.z -
                                vertex->normal.z * orthogonal_tangent.y;
    float perfect_bitangent_y = vertex->normal.z * orthogonal_tangent.x -
                                vertex->normal.x * orthogonal_tangent.z;
    float perfect_bitangent_z = vertex->normal.x * orthogonal_tangent.y -
                                vertex->normal.y * orthogonal_tangent.x;

    // dot(B_perfect, B_accumulated)
    float bitangent_alignment = perfect_bitangent_x * accumulated_bitangent->x +
                                perfect_bitangent_y * accumulated_bitangent->y +
                                perfect_bitangent_z * accumulated_bitangent->z;
    float handedness = (bitangent_alignment < 0.0f) ? -1.0f : 1.0f;
    vertex->tangent  = (vec4_t){
        orthogonal_tangent.x, orthogonal_tangent.y, orthogonal_tangent.z, handedness
    };
}

/**
 * Generates tangents for an entire mesh.
 *
 * Because adjacent triangles share vertices, this function guarantees that all triangles
 * get a chance to "vote" on a vertex's tangent direction before we finalize the math
 * in the second pass.
 *
 * @see https://terathon.com/blog/tangent-space.html
 *
 * @param vertices     Array of vertices making up the mesh. Tangent data will be written here.
 * @param vertex_count The total number of vertices in the array.
 * @param indices      Optional array of vertex indices (if using indexed drawing). Can be NULL.
 * @param index_count  The total number of indices. If 0, the mesh is assumed to be unindexed.
 */
void calculate_tangents(
    pak_vertex_t* vertices,
    uint32_t      vertex_count,
    uint32_t*     indices,
    uint32_t      index_count
) {
    for (uint32_t i = 0; i < vertex_count; i++) {
        vertices[i].tangent = (vec4_t){0.0f, 0.0f, 0.0f, 0.0f};
    }
    vec3_t*  bitangents     = calloc(vertex_count, sizeof(vec3_t));
    uint32_t triangle_count = (index_count > 0) ? (index_count / 3) : (vertex_count / 3);
    for (uint32_t triangle_index = 0; triangle_index < triangle_count; triangle_index++) {
        uint32_t index0 = (index_count > 0) ? indices[triangle_index * 3 + 0]
                                            : triangle_index * 3 + 0;
        uint32_t index1 = (index_count > 0) ? indices[triangle_index * 3 + 1]
                                            : triangle_index * 3 + 1;
        uint32_t index2 = (index_count > 0) ? indices[triangle_index * 3 + 2]
                                            : triangle_index * 3 + 2;
        accumulate_triangle_tangents(
            &vertices[index0],
            &vertices[index1],
            &vertices[index2],
            &bitangents[index0],
            &bitangents[index1],
            &bitangents[index2]
        );
    }
    for (uint32_t i = 0; i < vertex_count; i++) {
        orthogonalize_vertex_tangent(&vertices[i], &bitangents[i]);
    }
    free(bitangents);
}
