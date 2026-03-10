
#include <math.h>

#include "core/camera.h"
#include "core/math/mat4_math.h"

mat4_t mat4_identity() {
    mat4_t res     = {0};
    res.data[0][0] = 1.0f;
    res.data[1][1] = 1.0f;
    res.data[2][2] = 1.0f;
    res.data[3][3] = 1.0f;
    return res;
}

mat4_t mat4_rotate_y(float angle) {
    mat4_t res = mat4_identity();
    float  s   = sinf(angle);
    float  c   = cosf(angle);

    res.data[0][0] = c;
    res.data[0][2] = s; // For a right-handed system
    res.data[2][0] = -s;
    res.data[2][2] = c;

    return res;
}

mat4_t mat4_translate(vec3_t v) {
    mat4_t res     = mat4_identity();
    res.data[3][0] = v.x;
    res.data[3][1] = v.y;
    res.data[3][2] = v.z;
    return res;
}

// mat4_t mat4_identity() {
//     return (mat4_t){
//         {
//             {1.0f, 0.0f, 0.0f, 0.0f},
//             {0.0f, 1.0f, 0.0f, 0.0f},
//             {0.0f, 0.0f, 1.0f, 0.0f},
//             {0.0f, 0.0f, 0.0f, 1.0f},
//         },
//     };
// }

// Orthographic Projection: Fixes the Aspect Ratio
mat4_t mat4_ortho(float left, float right, float bottom, float top, float near, float far) {
    mat4_t res     = {0};
    res.data[0][0] = 2.0f / (right - left);
    res.data[1][1] = 2.0f / (top - bottom);
    res.data[2][2] = -2.0f / (far - near);
    res.data[3][0] = -(right + left) / (right - left);
    res.data[3][1] = -(top + bottom) / (top - bottom);
    res.data[3][2] = -(far + near) / (far - near);
    res.data[3][3] = 1.0f;
    return res;
}

mat4_t mat4_perspective(float fov_y, float aspect, float near, float far) {
    float  f       = 1.0f / tanf(fov_y / 2.0f);
    mat4_t res     = {0};
    res.data[0][0] = f / aspect;
    res.data[1][1] = f;
    res.data[2][2] = far / (near - far);
    res.data[2][3] = -1.0f;
    res.data[3][2] = (near * far) / (near - far);
    return res;
}

mat4_t mat4_mul(mat4_t a, mat4_t b) {
    mat4_t res = {0};
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            res.data[r][c] = a.data[r][0] * b.data[0][c] + a.data[r][1] * b.data[1][c] +
                             a.data[r][2] * b.data[2][c] + a.data[r][3] * b.data[3][c];
        }
    }
    return res;
}

mat4_t mat4_euler(float yaw, float pitch) {
    float y = yaw * (M_PI / 180.0f);
    float p = pitch * (M_PI / 180.0f);

    float sy = sinf(y);
    float cy = cosf(y);
    float sp = sinf(p);
    float cp = cosf(p);

    mat4_t res = mat4_identity();
    // Combined rotation matrix for Y (Yaw) then X (Pitch)
    res.data[0][0] = cy;
    res.data[0][1] = sy * sp;
    res.data[0][2] = sy * cp;
    res.data[1][1] = cp;
    res.data[1][2] = -sp;
    res.data[2][0] = -sy;
    res.data[2][1] = cy * sp;
    res.data[2][2] = cy * cp;
    return res;
}

mat4_t mat4_rotate_x(float angle) {
    mat4_t res = mat4_identity();
    float  s   = sinf(angle);
    float  c   = cosf(angle);

    res.data[1][1] = c;
    res.data[1][2] = -s;
    res.data[2][1] = s;
    res.data[2][2] = c;

    return res;
}

mat4_t mat4_view(camera_t cam) {
    float  p            = -cam.pitch * (M_PI / 180.0f);
    float  y            = -cam.yaw * (M_PI / 180.0f);
    mat4_t rot_x        = mat4_rotate_x(p);
    mat4_t rot_y        = mat4_rotate_y(y);
    mat4_t trans        = mat4_translate((vec3_t){-cam.pos.x, -cam.pos.y, -cam.pos.z});
    mat4_t combined_rot = mat4_mul(rot_x, rot_y);
    return mat4_mul(combined_rot, trans);
}
