#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef struct renderer_t renderer_t;
typedef struct physics_t physics_t;

typedef struct {
    bool is_running;
    renderer_t* renderer;
    physics_t* physics;
} engine_context_t;

#endif
