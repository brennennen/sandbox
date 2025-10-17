
#include "flattened_device_tree.h"

#include <stddef.h>
#include <stdint.h>


static uint32_t read_uint32_big_endian(uint8_t *bytes, int *index) {
    uint32_t value = 0;
    value |= (uint32_t)bytes[*index] << 24;
    value |= (uint32_t)bytes[*index + 1] << 16;
    value |= (uint32_t)bytes[*index + 2] << 8;
    value |= (uint32_t)bytes[*index + 3];
    *index += 4;
    return value;
}

static void fdt_read_header(fdt_header_t *header, uint8_t *bytes) {
    uint32_t index = 0;
    header->magic = read_uint32_big_endian(bytes, &index);
}

result_t fdt_read(flattened_device_tree_t *fdt, uint8_t *bytes) {
    fdt_read_header(&fdt->header, bytes);
}

result_t fdt_parse_file(flattened_device_tree_t *fdt, char *file_path) {
    // TODO: read file
    //fdt_read(&fdt->header, NULL);

}





