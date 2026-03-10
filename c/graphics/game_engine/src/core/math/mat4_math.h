#ifndef MAT4_MATH_H
#define MAT4_MATH_H

#include "core/camera.h"
#include "core/math/mat4_math.h"
#include "core/math/math_types.h"

mat4_t mat4_identity();
mat4_t mat4_translate(vec3_t v);
mat4_t mat4_rotate_y(float angle);
mat4_t mat4_perspective(float fov_y, float aspect, float near, float far);
mat4_t mat4_mul(mat4_t a, mat4_t b);
mat4_t mat4_view(camera_t cam);

#endif // MAT4_MATH_H
