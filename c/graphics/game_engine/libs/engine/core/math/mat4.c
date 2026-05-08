
#include <math.h>
#include <stdio.h>

#include "engine/core/camera.h"
#include "engine/core/math/mat4.h"

// Orthographic Projection: Fixes the Aspect Ratio
mat4_t mat4_ortho(float left, float right, float bottom, float top, float near, float far) {
    mat4_t res  = {0};
    res.m[0][0] = 2.0f / (right - left);
    res.m[1][1] = 2.0f / (top - bottom);
    res.m[2][2] = -2.0f / (far - near);
    res.m[3][0] = -(right + left) / (right - left);
    res.m[3][1] = -(top + bottom) / (top - bottom);
    res.m[3][2] = -(far + near) / (far - near);
    res.m[3][3] = 1.0f;
    return res;
}

mat4_t mat4_perspective(float fov_y, float aspect, float near, float far) {
    float  f    = 1.0f / tanf(fov_y / 2.0f);
    mat4_t res  = {0};
    res.m[0][0] = f / aspect;
    res.m[1][1] = f;
    res.m[2][2] = far / (near - far);
    res.m[2][3] = -1.0f;
    res.m[3][2] = (near * far) / (near - far);
    return res;
}

mat4_t mat4_mul(mat4_t a, mat4_t b) {
    mat4_t res = {0};
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            /* clang-format off */
            res.m[col][row] = a.m[0][row] * b.m[col][0] +
                              a.m[1][row] * b.m[col][1] +
                              a.m[2][row] * b.m[col][2] +
                              a.m[3][row] * b.m[col][3];
            /* clang-format on */
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
    res.m[0][0] = cy;
    res.m[0][1] = sy * sp;
    res.m[0][2] = sy * cp;
    res.m[1][1] = cp;
    res.m[1][2] = -sp;
    res.m[2][0] = -sy;
    res.m[2][1] = cy * sp;
    res.m[2][2] = cy * cp;
    return res;
}

mat4_t mat4_rotate_x(float angle) {
    mat4_t res = mat4_identity();
    float  s   = sinf(angle);
    float  c   = cosf(angle);

    res.m[1][1] = c;
    res.m[1][2] = -s;
    res.m[2][1] = s;
    res.m[2][2] = c;

    return res;
}

mat4_t mat4_rotate_y(float angle) {
    mat4_t res = mat4_identity();
    float  s   = sinf(angle);
    float  c   = cosf(angle);

    res.m[0][0] = c;
    res.m[0][2] = s; // For a right-handed system
    res.m[2][0] = -s;
    res.m[2][2] = c;

    return res;
}

mat4_t mat4_rotate_z(float angle) {
    mat4_t res = mat4_identity();
    float  s   = sinf(angle);
    float  c   = cosf(angle);

    res.m[0][0] = c;
    res.m[0][1] = s;
    res.m[1][0] = -s;
    res.m[1][1] = c;

    return res;
}

mat4_t mat4_look_at(vec3_t eye, vec3_t center, vec3_t up) {
    // Forward Vector (f)
    vec3_t f     = {center.x - eye.x, center.y - eye.y, center.z - eye.z};
    float  f_len = sqrtf(f.x * f.x + f.y * f.y + f.z * f.z);
    f.x /= f_len;
    f.y /= f_len;
    f.z /= f_len;

    // Right Vector (s = cross(f, up))
    vec3_t s     = {f.y * up.z - f.z * up.y, f.z * up.x - f.x * up.z, f.x * up.y - f.y * up.x};
    float  s_len = sqrtf(s.x * s.x + s.y * s.y + s.z * s.z);
    s.x /= s_len;
    s.y /= s_len;
    s.z /= s_len;

    // True Up Vector (u = cross(s, f))
    vec3_t u = {s.y * f.z - s.z * f.y, s.z * f.x - s.x * f.z, s.x * f.y - s.y * f.x};

    mat4_t res  = mat4_identity();
    res.m[0][0] = s.x;
    res.m[1][0] = s.y;
    res.m[2][0] = s.z;

    res.m[0][1] = u.x;
    res.m[1][1] = u.y;
    res.m[2][1] = u.z;

    res.m[0][2] = -f.x;
    res.m[1][2] = -f.y;
    res.m[2][2] = -f.z;

    res.m[3][0] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
    res.m[3][1] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    res.m[3][2] = (f.x * eye.x + f.y * eye.y + f.z * eye.z);
    res.m[3][3] = 1.0f;
    return res;
}

mat4_t mat4_scale(vec3_t s) {
    mat4_t res  = mat4_identity();
    res.m[0][0] = s.x;
    res.m[1][1] = s.y;
    res.m[2][2] = s.z;
    return res;
}

void mat4_print(const char* label, mat4_t m) {
    printf("%s:\n", label);
    for (int i = 0; i < 4; i++) {
        printf("  [ %f, %f, %f, %f ]\n", m.m[i][0], m.m[i][1], m.m[i][2], m.m[i][3]);
    }
    printf("\n");
}

mat4_t mat4_inverse(mat4_t mat) {
    const float* m = mat.elements;
    float        inv[16];
    float        det;

    // clang-format off
    inv[0] = m[5]  * m[10] * m[15] - m[5]  * m[11] * m[14] -
             m[9]  * m[6]  * m[15] + m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + m[4]  * m[11] * m[14] +
              m[8]  * m[6]  * m[15] - m[8]  * m[7]  * m[14] -
              m[12] * m[6]  * m[11] + m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - m[4]  * m[11] * m[13] -
             m[8]  * m[5] * m[15] + m[8]  * m[7] * m[13] +
             m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - m[8]  * m[6] * m[13] -
               m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + m[1]  * m[11] * m[14] +
              m[9]  * m[2] * m[15] - m[9]  * m[3] * m[14] -
              m[13] * m[2] * m[11] + m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - m[0]  * m[11] * m[14] -
             m[8]  * m[2] * m[15] + m[8]  * m[3] * m[14] +
             m[12] * m[2] * m[11] - m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + m[0]  * m[11] * m[13] +
              m[8]  * m[1] * m[15] - m[8]  * m[3] * m[13] -
              m[12] * m[1] * m[11] + m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - m[0]  * m[10] * m[13] -
              m[8]  * m[1] * m[14] + m[8]  * m[2] * m[13] +
              m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - m[1]  * m[7] * m[14] -
             m[5]  * m[2] * m[15] + m[5]  * m[3] * m[14] +
             m[13] * m[2] * m[7] - m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + m[0]  * m[7] * m[14] +
              m[4]  * m[2] * m[15] - m[4]  * m[3] * m[14] -
              m[12] * m[2] * m[7] + m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - m[0]  * m[7] * m[13] -
              m[4]  * m[1] * m[15] + m[4]  * m[3] * m[13] +
              m[12] * m[1] * m[7] - m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + m[0]  * m[6] * m[13] +
               m[4]  * m[1] * m[14] - m[4]  * m[2] * m[13] -
               m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] +
              m[5] * m[2] * m[11] - m[5] * m[3] * m[10] -
              m[9] * m[2] * m[7] + m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] -
             m[4] * m[2] * m[11] + m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] - m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] +
               m[4] * m[1] * m[11] - m[4] * m[3] * m[9] -
               m[8] * m[1] * m[7] + m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] -
              m[4] * m[1] * m[10] + m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    // clang-format on

    if (det == 0.0f)
        return mat4_identity();
    det = 1.0f / det;

    mat4_t res;
    for (int i = 0; i < 16; i++) {
        res.elements[i] = inv[i] * det;
    }

    return res;
}
