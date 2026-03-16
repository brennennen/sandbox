
#include <math.h>
#include <stdio.h>

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

mat4_t mat4_rotate_z(float angle) {
    mat4_t res = mat4_identity();
    float  s   = sinf(angle);
    float  c   = cosf(angle);

    res.data[0][0] = c;
    res.data[0][1] = s;
    res.data[1][0] = -s;
    res.data[1][1] = c;

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

    mat4_t res     = mat4_identity();
    res.data[0][0] = s.x;
    res.data[1][0] = s.y;
    res.data[2][0] = s.z;

    res.data[0][1] = u.x;
    res.data[1][1] = u.y;
    res.data[2][1] = u.z;

    res.data[0][2] = -f.x;
    res.data[1][2] = -f.y;
    res.data[2][2] = -f.z;

    res.data[3][0] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
    res.data[3][1] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    res.data[3][2] = (f.x * eye.x + f.y * eye.y + f.z * eye.z);
    res.data[3][3] = 1.0f;
    return res;
}

// mat4_t mat4_view(camera_t* cam) {
//     float yaw_rad   = cam->yaw * (M_PI / 180.0f);
//     float pitch_rad = cam->pitch * (M_PI / 180.0f);

//     // In a Z-up world, +Y is usually "Forward" into the screen.
//     vec3_t forward;
//     forward.x = sinf(yaw_rad) * cosf(pitch_rad);
//     forward.y = cosf(yaw_rad) * cosf(pitch_rad);
//     forward.z = sinf(pitch_rad);

//     vec3_t target   = {cam->pos.x + forward.x, cam->pos.y + forward.y, cam->pos.z + forward.z};
//     vec3_t world_up = {0.0f, 0.0f, 1.0f};

//     return mat4_look_at(cam.pos, target, world_up);
// }

void mat4_print(const char* label, mat4_t m) {
    printf("%s:\n", label);
    for (int i = 0; i < 4; i++) {
        printf("  [ %f, %f, %f, %f ]\n", m.data[i][0], m.data[i][1], m.data[i][2], m.data[i][3]);
    }
    printf("\n");
}
