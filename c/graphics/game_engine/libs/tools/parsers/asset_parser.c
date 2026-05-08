#include "asset_parser.h"
#include "engine/core/logger.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char* read_entire_file(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        log_error("Failed to open file: %s", filepath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

static bool get_next_token(char** stream, char* out_token, size_t max_len) {
    char* c = *stream;

    while (*c != '\0') {
        if (isspace(*c)) {
            c++;
        } else if (*c == '/' && *(c + 1) == '/') {
            while (*c != '\n' && *c != '\0')
                c++;
        } else {
            break;
        }
    }

    if (*c == '\0')
        return false;

    size_t len = 0;
    if (*c == '"') {
        c++;
        while (*c != '"' && *c != '\0' && len < max_len - 1) {
            out_token[len++] = *c++;
        }
        if (*c == '"')
            c++;
    }
    else if (*c == '{' || *c == '}' || *c == '=' || *c == ';') {
        out_token[len++] = *c++;
    }

    else {
        while (!isspace(*c) && *c != '{' && *c != '}' && *c != '=' && *c != ';' && *c != '\0' &&
               len < max_len - 1) {
            out_token[len++] = *c++;
        }
    }

    out_token[len] = '\0';
    *stream        = c;
    return true;
}

bool parse_model_file(const char* filepath, model_desc_t* out_model) {
    memset(out_model, 0, sizeof(model_desc_t));

    char* file_data = read_entire_file(filepath);
    if (!file_data)
        return false;

    char* stream = file_data;
    char  token[512];

    get_next_token(&stream, token, sizeof(token));
    if (strcmp(token, "Model") != 0) {
        log_error("Invalid file format. Expected 'Model', got '%s'", token);
        free(file_data);
        return false;
    }

    get_next_token(&stream, token, sizeof(token));
    if (strcmp(token, "{") != 0) {
        log_error("Expected '{' after Model");
        free(file_data);
        return false;
    }
    while (get_next_token(&stream, token, sizeof(token))) {
        if (strcmp(token, "}") == 0) {
            break;
        }

        char key[128];
        strcpy(key, token);
        get_next_token(&stream, token, sizeof(token));
        if (strcmp(token, "=") != 0) {
            log_error("Expected '=' after key '%s'", key);
            break;
        }
        get_next_token(&stream, token, sizeof(token));
        if (strcmp(key, "source_file") == 0) {
            strncpy(out_model->source_file, token, ASSET_MAX_PATH - 1);
        } else if (strcmp(key, "mesh_name") == 0) {
            strncpy(out_model->mesh_name, token, ASSET_MAX_NAME - 1);
        } else if (strcmp(key, "materials") == 0) {
            if (out_model->material_count < ASSET_MAX_MATERIALS) {
                strncpy(out_model->materials[out_model->material_count], token, ASSET_MAX_PATH - 1);
                out_model->material_count++;
            }
        } else {
            log_warn("Unknown key in .model file: '%s'", key);
        }

        get_next_token(&stream, token, sizeof(token));
    }

    free(file_data);
    return true;
}