#ifndef VEC3_H
#define VEC3_H

typedef struct {
    float x;
    float y;
    float z;
} vec3_t;

typedef struct {
    float data[4][4];
} mat4_t;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif
