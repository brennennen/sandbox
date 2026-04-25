#ifndef CAMERA_H
#define CAMERA_H

#include "shared/math_types.h"

typedef struct {
    vec3_t pos;
    float  yaw;
    float  pitch;
    float  fov;
} camera_t;

mat4_t camera_get_view_matrix(const camera_t* camera);

#endif // CAMERA_H
