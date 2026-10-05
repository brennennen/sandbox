
#include <stdio.h>
#include <string.h>

#include "path.h"

void path_extract_base_dir(const char* input_file, char* base_dir, size_t max_len) {
    snprintf(base_dir, max_len, ".");
    const char* last_slash     = strrchr(input_file, '/');
    const char* last_backslash = strrchr(input_file, '\\');
    const char* slash          = (last_slash > last_backslash) ? last_slash : last_backslash;

    if (slash) {
        size_t dir_len = slash - input_file;
        if (dir_len < max_len) {
            strncpy(base_dir, input_file, dir_len);
            base_dir[dir_len] = '\0';
        }
    }
}