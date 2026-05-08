#ifndef ASSET_PARSER_H
#define ASSET_PARSER_H

#include <stdbool.h>

#include "shared/asset_types.h"

bool parse_model_file(const char* filepath, model_desc_t* out_model);

bool parse_material_file(const char* filepath, material_desc_t* out_model);

#endif // ASSET_PARSER_H
