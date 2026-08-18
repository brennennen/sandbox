
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/core/logger.h"
#include "engine/platform/platform.h"

#include "pak_format.h"
#include "vfs.h"

#ifdef _WIN32
#define vfs_fseek _fseeki64
#define vfs_ftell _ftelli64
#else
#define vfs_fseek fseeko
#define vfs_ftell ftello
#endif

#define MAX_ARCHIVES 16
#define MAX_GLOBAL_CHUNKS 4096

#define WPAK_MAGIC 0x4B415057

#define MAX_ARCHIVES 16

typedef struct {
    char                    filepath[256];
    platform_file_mapping_t mapping; // Replaces data, size, and file_handle
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

    uint32_t archive_id = active_archive_count;

    // The OS handles the 9GB mapping instantly with zero mallocs!
    if (!platform_file_map_read(filepath, &active_archives[archive_id].mapping)) {
        log_error("VFS: Failed to memory map %s", filepath);
        return false;
    }

    uint32_t* magic = (uint32_t*)active_archives[archive_id].mapping.data;
    if (*magic != WPAK_MAGIC && *magic != 0x4B41505A) {
        log_error("VFS: Invalid Magic Number in %s (Found: 0x%X)", filepath, *magic);
        platform_file_unmap(&active_archives[archive_id].mapping);
        return false;
    }

    strncpy(
        active_archives[archive_id].filepath,
        filepath,
        sizeof(active_archives[archive_id].filepath) - 1
    );
    active_archives[archive_id].filepath[255] = '\0';

    active_archive_count++;

    log_info(
        "VFS: Successfully memory-mapped %s (%.2f MB)",
        filepath,
        (float)active_archives[archive_id].mapping.size / (1024.0f * 1024.0f)
    );

    return true;
}

// Since the file is memory mapped, we don't return a FILE*.
// We return a direct, zero-copy pointer to the chunk in virtual memory!
void* vfs_get_chunk_pointer(uint32_t chunk_id, uint64_t* out_size) {
    for (uint32_t i = 0; i < global_chunk_count; i++) {
        if (global_chunk_map[i].chunk_id == chunk_id) {
            uint32_t archive_id = global_chunk_map[i].archive_id;
            if (out_size)
                *out_size = global_chunk_map[i].byte_size;

            return (uint8_t*)active_archives[archive_id].mapping.data +
                   global_chunk_map[i].byte_offset;
        }
    }
    log_error("VFS: Chunk %d not found in any mounted archive!", chunk_id);
    return NULL;
}

void* vfs_get_mounted_archive_pointer(const char* filepath) {
    for (uint32_t i = 0; i < active_archive_count; i++) {
        if (strcmp(active_archives[i].filepath, filepath) == 0) {
            return active_archives[i].mapping.data;
        }
    }
    log_error("VFS: Archive not found for %s", filepath);
    return NULL;
}

void* vfs_read_bytes(uint64_t byte_offset, size_t size) {
    if (active_archive_count == 0) {
        log_error("VFS: No archives are currently mounted.");
        return NULL;
    }

    mounted_archive_t* archive = &active_archives[0];

    if (byte_offset + size > archive->mapping.size) {
        log_error("VFS: Read out of bounds (Offset: %llu, Size: %zu)", byte_offset, size);
        return NULL;
    }

    // Keep the malloc here for backwards compatibility if callers expect to free() this buffer.
    void* buffer = malloc(size);
    if (!buffer) {
        log_error("VFS: Out of memory allocating %zu bytes", size);
        return NULL;
    }

    memcpy(buffer, (uint8_t*)archive->mapping.data + byte_offset, size);
    return buffer;
}

void vfs_shutdown(void) {
    for (uint32_t i = 0; i < active_archive_count; i++) {
        platform_file_unmap(&active_archives[i].mapping);
    }
    active_archive_count = 0;
    log_info("VFS: Shut down and memory maps released.");
}
