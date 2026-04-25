#ifndef SCENE_PARSER_H
#define SCENE_PARSER_H

#include <stdbool.h>

#include "shared/scene_types.h"

char* read_file_to_string(const char* filepath);
bool  parse_scene_file(const char* text, const char* base_dir, scene_desc_t* out_scene);

#endif // SCENE_PARSER_H
