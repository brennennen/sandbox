#include "core/camera.h"
#include "core/math/mat4_math.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

mat4_t camera_get_view_matrix(const camera_t* camera) {
    float  yaw_rad   = camera->yaw * (M_PI / 180.0f);
    float  pitch_rad = camera->pitch * (M_PI / 180.0f);
    vec3_t forward; // +Z-up, +Y-forward
    forward.x     = sinf(yaw_rad) * cosf(pitch_rad);
    forward.y     = cosf(yaw_rad) * cosf(pitch_rad);
    forward.z     = sinf(pitch_rad);
    vec3_t target = {
        camera->pos.x + forward.x,
        camera->pos.y + forward.y,
        camera->pos.z + forward.z,
    };
    vec3_t world_up = {0.0f, 0.0f, 1.0f};
    return mat4_look_at(camera->pos, target, world_up);
}
