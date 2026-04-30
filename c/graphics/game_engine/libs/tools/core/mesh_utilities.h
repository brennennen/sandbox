#ifndef MESH_UTILITIES_H
#define MESH_UTILITIES_H

#include <math.h>

#include "shared/scene_types.h"

void accumulate_triangle_tangents(
    pak_vertex_t* vertex0,
    pak_vertex_t* vertex1,
    pak_vertex_t* vertex2,
    vec3_t*       bitangent0,
    vec3_t*       bitangent1,
    vec3_t*       bitangent2
);

void orthogonalize_vertex_tangent(pak_vertex_t* vertex, vec3_t* accumulated_bitangent);

void calculate_tangents(
    pak_vertex_t* vertices,
    uint32_t      vertex_count,
    uint32_t*     indices,
    uint32_t      index_count
);

// TODO: move the function below into a core/common library?

/**
 * Forces a target vector to sit at a perfect 90-degree angle to a baseline normal vector.
 *
 * If a vector gets bent or skewed out of alignment, the Gram-Schmidt process shaves off
 * the overlapping parts to snap it back to a perfect right angle. It then normalizes the
 * result so its length is exactly 1.0.
 *
 * @see
 https://www.khanacademy.org/math/linear-algebra/alternate-bases/orthonormal-basis/v/linear-algebra-the-gram-schmidt-process
 * @see "Introduction to 3D Game Programming With DirectX 11" Chapter 18 Normal Mapping and
 Displacement Mapping
 *
 * @param normal The perfectly straight baseline vector (this MUST already have a length of 1.0).
 * @param target The messy target vector that needs to be straightened.
 * @return  A new vector that is perfectly perpendicular to 'normal' with a length of 1.0.
 */
static inline vec3_t vec3_gram_schmidt(vec3_t normal, vec3_t target) {
    // Calculate how much the target overlaps with the normal (Dot Product)
    float projection_amount = normal.x * target.x + normal.y * target.y + normal.z * target.z;

    // Subtract the overlapping amount to get the purely orthogonal direction
    vec3_t orthogonal_result;
    orthogonal_result.x = target.x - (normal.x * projection_amount);
    orthogonal_result.y = target.y - (normal.y * projection_amount);
    orthogonal_result.z = target.z - (normal.z * projection_amount);

    // Normalize the newly straightened vector so its length is exactly 1.0
    float vector_length = sqrtf(
        orthogonal_result.x * orthogonal_result.x + orthogonal_result.y * orthogonal_result.y +
        orthogonal_result.z * orthogonal_result.z
    );

    if (vector_length > 0.0001f) {
        orthogonal_result.x /= vector_length;
        orthogonal_result.y /= vector_length;
        orthogonal_result.z /= vector_length;
    } else {
        // Safe fallback
        orthogonal_result.x = 1.0f;
        orthogonal_result.y = 0.0f;
        orthogonal_result.z = 0.0f;
    }

    return orthogonal_result;
}

#endif // MESH_UTILITIES_H
