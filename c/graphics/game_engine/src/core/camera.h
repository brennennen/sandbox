#ifndef CAMERA_H
#define CAMERA_H

#include "core/math/math_types.h"

typedef struct {
    vec3_t pos;
    float  yaw;
    float  pitch;
    float  fov;
} camera_t;

#endif // CAMERA_H
