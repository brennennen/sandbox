#pragma once
#include <stdbool.h>

/**
 * Extracts a monolithic glTF file into a collection of intermediate text assets.
 * @param gltf_path The source file.
 * @param output_dir The directory to dump the generated files.
 * @return true on success.
 */
bool extract_gltf_monolithic(const char* gltf_path, const char* output_dir);
