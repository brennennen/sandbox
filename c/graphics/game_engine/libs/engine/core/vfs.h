#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint32_t chunk_id;
    uint32_t archive_id;
    uint64_t byte_offset;
    uint64_t byte_size;
} chunk_locator_t;

bool vfs_mount_archive(const char* filepath);

FILE* vfs_open_chunk(uint32_t chunk_id, uint64_t* out_offset);

void* vfs_read_bytes(uint64_t byte_offset, size_t size);

void* vfs_get_mounted_archive_pointer(const char* filepath);

void vfs_shutdown(void);
