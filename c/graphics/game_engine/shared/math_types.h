#ifndef MATH_TYPES_H
#define MATH_TYPES_H

typedef union {
    float data[3];
    struct {
        float x;
        float y;
        float z;
    };
} vec3_t;

typedef union {
    float data[4];
    struct {
        float x;
        float y;
        float z;
        float w;
    };
} vec4_t;

typedef union {
    float  data[4][4];
    float  elements[16];
    vec4_t columns[4];
} mat4_t;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif // MATH_TYPES_H
