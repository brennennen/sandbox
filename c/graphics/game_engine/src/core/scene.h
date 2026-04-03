#ifndef SCENE_H
#define SCENE_H

#include "modules/graphics/graphics_types.h"

#define MAX_SCENE_OBJECTS 4096

typedef struct {
    render_object_t objects[MAX_SCENE_OBJECTS];
    uint32_t        object_count;
} scene_t;

#endif
