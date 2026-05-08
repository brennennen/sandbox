#ifndef MAT4_MATH_H
#define MAT4_MATH_H

#include <math.h>

#include "engine/core/camera.h"
#include "engine/core/math/mat4.h"
#include "math_types.h"
#include "shared/math_types.h"

static inline mat4_t mat4_identity() {
    return (mat4_t){
        {
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        },
    };
}

static inline mat4_t mat4_translate(vec3_t v) {
    mat4_t res  = mat4_identity();
    res.m[3][0] = v.x;
    res.m[3][1] = v.y;
    res.m[3][2] = v.z;
    return res;
}

mat4_t mat4_rotate_x(float angle);
mat4_t mat4_rotate_y(float angle);
mat4_t mat4_rotate_z(float angle);
mat4_t mat4_look_at(vec3_t eye, vec3_t center, vec3_t up);
mat4_t mat4_perspective(float fov_y, float aspect, float near, float far);
mat4_t mat4_mul(mat4_t a, mat4_t b);
// mat4_t mat4_view(camera_t* cam);
mat4_t mat4_scale(vec3_t s);
void   mat4_print(const char* label, mat4_t m);

/**
 * Finds the determinant of a 2x2 matrix.
 * @see https://en.wikipedia.org/wiki/Determinant
 */
static inline float _mat4_determinant_2x2(float a, float b, float c, float d) {
    return (a * d) - (b * c);
}

/**
 * Calculates a cofactor 3x3 matrix to keep surface directions (normals) accurate.
 *
 * Used in lighting math to "un-bend" bent surface normals, so they always remain
 * perpendicular to the surface geometry.
 *
 * @param transform The original 4x4 transformation matrix of the 3D object.
 * @return A 3x3 cofactor matrix designed exclusively for transforming normal vectors.
 */
static inline mat3_t mat4_get_normal_matrix(mat4_t transform) {
    // clang-format off
    float m00 = transform.m[0][0]; float m10 = transform.m[1][0]; float m20 = transform.m[2][0];
    float m01 = transform.m[0][1]; float m11 = transform.m[1][1]; float m21 = transform.m[2][1];
    float m02 = transform.m[0][2]; float m12 = transform.m[1][2]; float m22 = transform.m[2][2];

    mat3_t c;
    c.m[0][0] = _mat4_determinant_2x2(m11, m12, m21, m22);
    c.m[0][1] = -1 * _mat4_determinant_2x2(m01, m02, m21, m22);
    c.m[0][2] = _mat4_determinant_2x2(m01, m02, m11, m12);

    c.m[1][0] = -1 * _mat4_determinant_2x2(m10, m12, m20, m22);
    c.m[1][1] = _mat4_determinant_2x2(m00, m02, m20, m22);
    c.m[1][2] = -1 * _mat4_determinant_2x2(m00, m02, m10, m12);

    c.m[2][0] = _mat4_determinant_2x2(m10, m11, m20, m21);
    c.m[2][1] = -1 * _mat4_determinant_2x2(m00, m01, m20, m21);
    c.m[2][2] = _mat4_determinant_2x2(m00, m01, m10, m11);
    return c;
    //clang-format on
}

/**
 * Transforms a 3D point in space.
 * Applies rotation, scaling, and translation. Assumes the vector has an implicit
 * 'w' component of 1.0.
 */
static inline vec3_t mat4_transform_point(mat4_t m, vec3_t pos) {
    return (vec3_t){
        pos.x * m.m[0][0] + pos.y * m.m[1][0] + pos.z * m.m[2][0] + m.m[3][0],
        pos.x * m.m[0][1] + pos.y * m.m[1][1] + pos.z * m.m[2][1] + m.m[3][1],
        pos.x * m.m[0][2] + pos.y * m.m[1][2] + pos.z * m.m[2][2] + m.m[3][2]
    };
}

/**
 * Transforms a surface normal and re-normalizes it.
 * Uses a 3x3 cofactor/normal matrix. Ignores translation entirely.
 */
static inline vec3_t mat3_transform_normal(mat3_t c, vec3_t norm) {
    vec3_t res = {
        norm.x * c.m[0][0] + norm.y * c.m[0][1] + norm.z * c.m[0][2],
        norm.x * c.m[1][0] + norm.y * c.m[1][1] + norm.z * c.m[1][2],
        norm.x * c.m[2][0] + norm.y * c.m[2][1] + norm.z * c.m[2][2]
    };

    float len = sqrtf(res.x * res.x + res.y * res.y + res.z * res.z);
    if (len > 0.00001f) {
        res.x /= len;
        res.y /= len;
        res.z /= len;
    }
    return res;
}

/**
 * Transforms a tangent vector, normalizes it, and preserves handedness.
 * Applies rotation and scaling but ignores translation (implicit 'w' of 0.0
 * for the xyz transformation). The original 'w' component is passed through untouched
 * for bitangent calculation in the shader.
 */
static inline vec4_t mat4_transform_tangent(mat4_t m, vec4_t tangent) {
    vec3_t res = {
        tangent.x * m.m[0][0] + tangent.y * m.m[1][0] + tangent.z * m.m[2][0],
        tangent.x * m.m[0][1] + tangent.y * m.m[1][1] + tangent.z * m.m[2][1],
        tangent.x * m.m[0][2] + tangent.y * m.m[1][2] + tangent.z * m.m[2][2]
    };

    float len = sqrtf(res.x * res.x + res.y * res.y + res.z * res.z);
    if (len > 0.00001f) {
        res.x /= len;
        res.y /= len;
        res.z /= len;
    }
    // Preserve the 'w' handedness component
    return (vec4_t){res.x, res.y, res.z, tangent.w};
}

/**
 * Computes the inverse of a 4x4 transformation matrix.
 *
 * If a matrix transforms a point from Space A to Space B, the inverse matrix will
 * transform a point from Space B back to Space A. Implements the "Adjugate Matrix Formula"
 *
 * @see https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
 *
 * @param mat The 4x4 matrix to invert.
 * @return The inverted 4x4 matrix, or an Identity matrix if the input cannot be inverted.
 */
mat4_t mat4_inverse(mat4_t mat);


static inline float vec3_dot(vec3_t a, vec3_t b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

static inline float vec4_dot(vec4_t a, vec4_t b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

static inline vec4_t mat4_mul_vec4(mat4_t m, vec4_t v) {
    return (vec4_t){
        m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0] * v.w,
        m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1] * v.w,
        m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2] * v.w,
        m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3] * v.w
    };
}

#endif // MAT4_MATH_H
