
#include "engine/debug/freefly_camera.h"
#include "engine/core/camera.h"
#include "engine/platform/platform.h"

#include <math.h>

void freefly_camera_update(camera_t* camera, platform_t* platform, float delta_time) {
    float dx;
    float dy;

    platform_get_mouse_delta(platform, &dx, &dy);

    float sensitivity = 0.1f;
    camera->yaw += dx * sensitivity;
    camera->pitch -= dy * sensitivity;

    if (camera->pitch > 89.0f) {
        camera->pitch = 89.0f;
    }
    if (camera->pitch < -89.0f) {
        camera->pitch = -89.0f;
    }

    const float base_cam_speed = 5.0f;
    float       cam_speed      = base_cam_speed * delta_time;

    float yaw_rad   = camera->yaw * (M_PI / 180.0f);
    float forward_x = sinf(yaw_rad);
    float forward_y = cosf(yaw_rad);
    float right_x   = cosf(yaw_rad);
    float right_y   = -sinf(yaw_rad);

    if (platform_get_key(platform, KEY_W)) {
        camera->pos.x += forward_x * cam_speed;
        camera->pos.y += forward_y * cam_speed;
    }
    if (platform_get_key(platform, KEY_S)) {
        camera->pos.x -= forward_x * cam_speed;
        camera->pos.y -= forward_y * cam_speed;
    }
    if (platform_get_key(platform, KEY_A)) {
        camera->pos.x -= right_x * cam_speed;
        camera->pos.y -= right_y * cam_speed;
    }
    if (platform_get_key(platform, KEY_D)) {
        camera->pos.x += right_x * cam_speed;
        camera->pos.y += right_y * cam_speed;
    }

    if (platform_get_key(platform, KEY_SPACE))
        camera->pos.z += cam_speed;
    if (platform_get_key(platform, KEY_LSHIFT))
        camera->pos.z -= cam_speed;
}
