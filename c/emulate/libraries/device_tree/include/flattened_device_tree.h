#ifndef FLATTENED_DEVICE_TREE_H
#define FLATTENED_DEVICE_TREE_H

#include <stdint.h>

#include "shared/include/result.h"

typedef struct {
    char name[31];  // 2.2.1 Node Names

} node_t;

/**
 * Flattened Device Tree Header. Big-endian.
 */
typedef struct fdt_header {
    uint32_t magic;      // hard coded magic: 0xd00dfeed
    uint32_t totalsize;  // total size in bytes. includes all sections: header, mem reservation
                         // block, structure block, string blocks, and any free space/alignment gaps
                         // between blocks.
    uint32_t
        off_dt_struct;  // offset in bytes to the struct section from the beginning of the header.
    uint32_t
        off_dt_strings;  // offset in bytes to the strings section from the beginning of the header.
    uint32_t off_mem_rsvmap;  // offset in bytes to the memory reservation block from the beginning
                              // of the header.
    uint32_t version;  // version of the devicetree data structre. 17 is latest as of writing this.
    uint32_t last_comp_version;  // last backwards compatible version (17 is compatible with 16, so
                                 // 16 if authoring a ver 17 file).
    uint32_t boot_cpuid_phys;    // physical id of the system's boot cpu.
    uint32_t size_dt_strings;    // length in bytes of the strings section
    uint32_t size_dt_struct;     // length in bytes of the struct section
} fdt_header_t;

typedef struct {
    fdt_header_t header;
    uint8_t* blob_data;
} flattened_device_tree_t;

result_t fdt_parse_file(flattened_device_tree_t* fdt, char* file_path);

#endif  // FLATTENED_DEVICE_TREE_H
