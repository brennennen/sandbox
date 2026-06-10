
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/core/logger.h"

#include "pak_format.h"
#include "vfs.h"

#define MAX_ARCHIVES 16
#define MAX_GLOBAL_CHUNKS 4096

#define WPAK_MAGIC 0x4B415057

#define MAX_ARCHIVES 16

typedef struct {
    char   filepath[256];
    FILE*  file_handle;
    void*  data; // Pointer to the start of the file in RAM
    size_t size; // Total bytes
} mounted_archive_t;

static mounted_archive_t active_archives[MAX_ARCHIVES];
static uint32_t          active_archive_count = 0;

// The Master Routing Table
static chunk_locator_t global_chunk_map[MAX_GLOBAL_CHUNKS];
static uint32_t        global_chunk_count = 0;

bool vfs_mount_archive(const char* filepath) {
    if (active_archive_count >= MAX_ARCHIVES) {
        log_error("VFS: Max archives reached.");
        return false;
    }

    FILE* file = fopen(filepath, "rb");
    if (!file) {
        log_error("VFS: Could not open %s", filepath);
        return false;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    void* buffer = malloc(file_size);
    if (!buffer) {
        log_error("VFS: Out of memory allocating %zu bytes for %s", file_size, filepath);
        fclose(file);
        return false;
    }

    fread(buffer, 1, file_size, file);
    fclose(file);

    uint32_t* magic = (uint32_t*)buffer;
    if (*magic != WPAK_MAGIC) {
        log_error("VFS: Invalid Magic Number in %s (Found: 0x%X)", filepath, *magic);
        free(buffer);
        return false;
    }

    uint32_t archive_id = active_archive_count++;

    strncpy(
        active_archives[archive_id].filepath,
        filepath,
        sizeof(active_archives[archive_id].filepath) - 1
    );
    active_archives[archive_id].filepath[255] = '\0';

    active_archives[archive_id].data = buffer;
    active_archives[archive_id].size = file_size;

    log_info(
        "VFS: Successfully mounted %s (%.2f MB)", filepath, (float)file_size / (1024.0f * 1024.0f)
    );
    return true;
}

FILE* vfs_open_chunk(uint32_t chunk_id, uint64_t* out_offset) {
    for (uint32_t i = 0; i < global_chunk_count; i++) {
        if (global_chunk_map[i].chunk_id == chunk_id) {
            *out_offset = global_chunk_map[i].byte_offset;
            return active_archives[global_chunk_map[i].archive_id].file_handle;
        }
    }
    log_error("VFS: Chunk %d not found in any mounted archive!", chunk_id);
    return NULL;
}

void* vfs_get_mounted_archive_pointer(const char* filepath) {
    for (uint32_t i = 0; i < active_archive_count; i++) {
        if (strcmp(active_archives[i].filepath, filepath) == 0) {
            return active_archives[i].data;
        }
    }
    log_error("VFS: Archive not found for %s", filepath);
    return NULL;
}

void* vfs_read_bytes(uint64_t byte_offset, size_t size) {
    // Note: Assuming Archive 0 is the primary mounted PAK.
    // If multiple PAKs are mounted, pak_texture_t will eventually need an `archive_id` field.
    if (active_archive_count == 0) {
        log_error("VFS: No archives are currently mounted.");
        return NULL;
    }

    mounted_archive_t* archive = &active_archives[0];

    if (byte_offset + size > archive->size) {
        log_error("VFS: Read out of bounds (Offset: %llu, Size: %zu)", byte_offset, size);
        return NULL;
    }

    void* buffer = malloc(size);
    if (!buffer) {
        log_error("VFS: Out of memory allocating %zu bytes", size);
        return NULL;
    }

    memcpy(buffer, (uint8_t*)archive->data + byte_offset, size);
    return buffer;
}

void vfs_shutdown(void) {
    for (uint32_t i = 0; i < active_archive_count; i++) {
        if (active_archives[i].data) {
            free(active_archives[i].data);
            active_archives[i].data = NULL;
        }
    }
    active_archive_count = 0;
    log_info("VFS: Shut down and memory cleared.");
}
