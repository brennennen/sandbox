#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "engine/core/math/mat4.h"

typedef struct {
    vec4_t planes[6]; // Left, Right, Top, Bottom, Near, Far
} frustum_t;

// normalizes a plane so that we can calculate accurate distances
static inline vec4_t plane_normalize(vec4_t p) {
    float length = sqrtf(p.x * p.x + p.y * p.y + p.z * p.z);
    if (length > 0.00001f) {
        return (vec4_t){p.x / length, p.y / length, p.z / length, p.w / length};
    }
    return p;
}

/**
 * Extracts the 6 frustum planes from a Column-Major View-Projection matrix.
 * Assumes Vulkan's clip space (Z goes from 0 to 1).
 */
static inline frustum_t frustum_extract(mat4_t vp) {
    vec4_t r0 = {vp.m[0][0], vp.m[1][0], vp.m[2][0], vp.m[3][0]};
    vec4_t r1 = {vp.m[0][1], vp.m[1][1], vp.m[2][1], vp.m[3][1]};
    vec4_t r2 = {vp.m[0][2], vp.m[1][2], vp.m[2][2], vp.m[3][2]};
    vec4_t r3 = {vp.m[0][3], vp.m[1][3], vp.m[2][3], vp.m[3][3]};

    frustum_t f;
    f.planes[0] = plane_normalize((vec4_t){r3.x + r0.x, r3.y + r0.y, r3.z + r0.z, r3.w + r0.w});
    f.planes[1] = plane_normalize((vec4_t){r3.x - r0.x, r3.y - r0.y, r3.z - r0.z, r3.w - r0.w});
    f.planes[2] = plane_normalize((vec4_t){r3.x + r1.x, r3.y + r1.y, r3.z + r1.z, r3.w + r1.w});
    f.planes[3] = plane_normalize((vec4_t){r3.x - r1.x, r3.y - r1.y, r3.z - r1.z, r3.w - r1.w});
    f.planes[4] = plane_normalize(r2);
    f.planes[5] = plane_normalize((vec4_t){r3.x - r2.x, r3.y - r2.y, r3.z - r2.z, r3.w - r2.w});
    return f;
}

/**
 * Tests if a sphere is completely outside the frustum.
 * @return true if visible, false if culled.
 */
static inline bool frustum_test_sphere(frustum_t* f, vec3_t center, float radius) {
    for (int i = 0; i < 6; i++) {
        vec3_t normal   = {f->planes[i].x, f->planes[i].y, f->planes[i].z};
        float  distance = vec3_dot(normal, center) + f->planes[i].w;
        if (distance < -radius) {
            return false;
        }
    }
    return true;
}

#endif
