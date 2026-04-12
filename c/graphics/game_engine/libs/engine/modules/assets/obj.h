#ifndef OBJ_H
#define OBJ_H

#include <stdbool.h>
#include <stdint.h>

#include "engine/modules/graphics/graphics_types.h"

// v/vt/vn format.
bool load_obj(const char* filepath, mesh_data_t* out_mesh);
void free_mesh(mesh_data_t* mesh);

#endif
