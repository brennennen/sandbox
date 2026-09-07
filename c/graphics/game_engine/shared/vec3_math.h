#ifndef VEC3_MATH_H
#define VEC3_MATH_H

#include "shared/math_types.h"
#include <math.h>

static inline vec3_t vec3_add(vec3_t a, vec3_t b) {
    return (vec3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline vec3_t vec3_sub(vec3_t a, vec3_t b) {
    return (vec3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline vec3_t vec3_mul(vec3_t a, vec3_t b) {
    return (vec3_t){a.x * b.x, a.y * b.y, a.z * b.z};
}

static inline vec3_t vec3_scale(vec3_t v, float s) { return (vec3_t){v.x * s, v.y * s, v.z * s}; }

static inline float vec3_length_squared(vec3_t v) {
    return (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
}

static inline float vec3_length(vec3_t v) { return sqrtf(vec3_length_squared(v)); }

static inline vec3_t vec3_normalize(vec3_t v) {
    float len = vec3_length(v);
    if (len > 0.000001f) {
        float inv_len = 1.0f / len;
        return (vec3_t){v.x * inv_len, v.y * inv_len, v.z * inv_len};
    }
    return (vec3_t){0.0f, 0.0f, 0.0f};
}

static inline float vec3_dot(vec3_t a, vec3_t b) { return (a.x * b.x) + (a.y * b.y) + (a.z * b.z); }

static inline vec3_t vec3_cross(vec3_t a, vec3_t b) {
    return (vec3_t){(a.y * b.z) - (a.z * b.y),
                    (a.z * b.x) - (a.x * b.z),
                    (a.x * b.y) - (a.y * b.x)};
}

static inline vec3_t vec3_invert(vec3_t v) { return (vec3_t){-v.x, -v.y, -v.z}; }

static inline vec3_t vec3_lerp(vec3_t a, vec3_t b, float t) {
    return (vec3_t){a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t};
}

#endif // VEC3_MATH_H
