
#ifndef FREEFLY_CAMERA_H
#define FREEFLY_CAMERA_H

#include "engine/core/camera.h"
#include "engine/platform/platform.h"

void freefly_camera_update(camera_t* camera, platform_t* platform, float delta_time);

#endif // FREEFLY_CAMERA_H
