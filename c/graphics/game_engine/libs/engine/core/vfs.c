
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "engine/core/logger.h"

#include "vfs.h"
#include "pak_format.h"

#define MAX_ARCHIVES 16
#define MAX_GLOBAL_CHUNKS 4096

static mounted_archive_t active_archives[MAX_ARCHIVES];
static uint32_t          active_archive_count = 0;

// The Master Routing Table
static chunk_locator_t global_chunk_map[MAX_GLOBAL_CHUNKS];
static uint32_t        global_chunk_count = 0;

bool vfs_mount_archive(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        log_error("VFS: Could not open %s", filepath);
        return false;
    }

    uint32_t archive_id                     = active_archive_count++;
    active_archives[archive_id].archive_id  = archive_id;
    active_archives[archive_id].file_handle = file;

    pak_header_t header;
    fread(&header, sizeof(pak_header_t), 1, file);

    if (header.magic != PAK_MAGIC) {
        log_error("VFS: Invalid Magic Number in %s", filepath);
        return false;
    }

    // TODO: add a chunk offset table and support multiple chunks
    global_chunk_map[global_chunk_count].chunk_id    = 0; // Hardcoded to chunk 0
    global_chunk_map[global_chunk_count].archive_id  = archive_id;
    global_chunk_map[global_chunk_count].byte_offset = sizeof(pak_header_t);
    global_chunk_count++;

    log_info("VFS: Mounted %s", filepath);
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
