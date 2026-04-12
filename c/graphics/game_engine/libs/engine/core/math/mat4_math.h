#ifndef MAT4_MATH_H
#define MAT4_MATH_H

#include "engine/core/camera.h"
#include "engine/core/math/mat4_math.h"
#include "engine/core/math/math_types.h"

mat4_t mat4_identity();
mat4_t mat4_translate(vec3_t v);
mat4_t mat4_rotate_x(float angle);
mat4_t mat4_rotate_y(float angle);
mat4_t mat4_rotate_z(float angle);
mat4_t mat4_look_at(vec3_t eye, vec3_t center, vec3_t up);
mat4_t mat4_perspective(float fov_y, float aspect, float near, float far);
mat4_t mat4_mul(mat4_t a, mat4_t b);
// mat4_t mat4_view(camera_t* cam);
mat4_t mat4_scale(vec3_t s);
void   mat4_print(const char* label, mat4_t m);

#endif // MAT4_MATH_H
