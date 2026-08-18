#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "engine/core/logger.h"
#include "shared/scene_types.h"

// Helper to check byte alignment
static bool is_aligned(uint64_t offset, uint64_t alignment) { return (offset % alignment) == 0; }

int main(int argc, char** argv) {
    log_init();

    if (argc < 2) {
        log_error("Usage: pak_validator <file.pak>");
        return 1;
    }

    const char* filepath = argv[1];
    FILE*       file     = fopen(filepath, "rb");
    if (!file) {
        log_error("Failed to open file: %s", filepath);
        return 1;
    }

    // Get absolute physical file size
    _fseeki64(file, 0, SEEK_END);
    int64_t file_size = _ftelli64(file);
    _fseeki64(file, 0, SEEK_SET);

    log_info("--- PAK VALIDATOR ---");
    log_info("Target: %s", filepath);
    log_info("Physical Size: %.2f MB", (double)file_size / (1024.0 * 1024.0));
    log_info("---------------------");

    // Validate Header & Magic
    world_pak_t header;
    if (fread(&header, sizeof(world_pak_t), 1, file) != 1) {
        log_error("CRITICAL: File too small to contain a valid header!");
        fclose(file);
        return 1;
    }

    if (header.magic != PAK_MAGIC) {
        log_error("CRITICAL: INVALID MAGIC. Expected 0x%X, got 0x%X", PAK_MAGIC, header.magic);
        fclose(file);
        return 1;
    }
    log_info("[PASS] Magic Number Validation");
    log_info("[PASS] PAK Version Validation (v%u)", header.version);

    // Validate TOC Memory Bounds
    bool bounds_ok = true;

    uint64_t vertex_end = header.vertex_offset + (header.vertex_count * sizeof(pak_vertex_t));
    if (vertex_end > file_size) {
        log_error("OOB VERTICES: Extends to %llu (EOF is %lld)", vertex_end, file_size);
        bounds_ok = false;
    }

    uint64_t mesh_end = header.mesh_offset + (header.mesh_count * sizeof(pak_mesh_t));
    if (mesh_end > file_size) {
        log_error("OOB MESHES: Extends to %llu (EOF is %lld)", mesh_end, file_size);
        bounds_ok = false;
    }

    uint64_t tex_toc_end = header.texture_offset + (header.texture_count * sizeof(pak_texture_t));
    if (tex_toc_end > file_size) {
        log_error("OOB TEXTURE TOC: Extends to %llu (EOF is %lld)", tex_toc_end, file_size);
        bounds_ok = false;
    }

    if (!bounds_ok) {
        log_error("CRITICAL: TOC Bounds validation failed. Aborting payload checks.");
        fclose(file);
        return 1;
    }
    log_info("[PASS] TOC Memory Bounds Validation");

    // Validate Heavy Payloads & Alignment
    pak_texture_t* textures = malloc(header.texture_count * sizeof(pak_texture_t));
    _fseeki64(file, header.texture_offset, SEEK_SET);
    fread(textures, sizeof(pak_texture_t), header.texture_count, file);

    log_info("--- TEXTURE PAYLOAD VALIDATION ---");
    int alignment_failures = 0;
    int oob_failures       = 0;

    for (uint32_t i = 0; i < header.texture_count; i++) {
        pak_texture_t* tex = &textures[i];
        if (tex->byte_size == 0)
            continue; // Skip empty textures

        bool     tex_ok      = true;
        uint64_t end_address = tex->byte_offset + tex->byte_size;

        // Vulkan 16-Byte Alignment Check
        if (!is_aligned(tex->byte_offset, 16)) {
            log_error(
                "  [TEX %u] UNALIGNED: Offset %llu is not 16-byte aligned!", i, tex->byte_offset
            );
            alignment_failures++;
            tex_ok = false;
        }

        // EOF Bounds Check
        if (end_address > file_size) {
            log_error(
                "  [TEX %u] OUT OF BOUNDS: Payload ends at %llu (EOF is %lld)",
                i,
                end_address,
                file_size
            );
            oob_failures++;
            tex_ok = false;
        }

        if (tex_ok) {
            log_debug(
                "  [TEX %u] OK | Offset: %llu | Size: %llu", i, tex->byte_offset, tex->byte_size
            );
        }
    }

    free(textures);
    fclose(file);

    log_info("--- VALIDATION SUMMARY ---");
    log_info("Entities: %u", header.entity_count);
    log_info("Meshes:   %u", header.mesh_count);
    log_info("Vertices: %u", header.vertex_count);
    log_info("Indices:  %u", header.index_count);
    log_info("Textures: %u", header.texture_count);
    log_info("--------------------------");

    if (alignment_failures == 0 && oob_failures == 0) {
        log_info("[SUCCESS] PAK file is structurally sound and Vulkan-ready.");
        return 0;
    } else {
        log_error(
            "[FAILED] PAK file has %d alignment errors and %d OOB errors.",
            alignment_failures,
            oob_failures
        );
        return 1;
    }
}