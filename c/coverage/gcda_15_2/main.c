/**
 * gcc ./main.c
 * ./a.out
 * gcov-dump ./test.gcda
 */

#include <stdint.h>
#include <stdio.h>

// Writes a 32-bit integer as 4 little-endian bytes
int write_uint32(uint8_t* buffer, uint32_t buffer_size, uint16_t* index, uint32_t number) {
    if (*index + 4 > buffer_size) {
        return -1;  // Buffer overflow
    }
    buffer[(*index)++] = (uint8_t)(number & 0xFF);
    buffer[(*index)++] = (uint8_t)((number >> 8) & 0xFF);
    buffer[(*index)++] = (uint8_t)((number >> 16) & 0xFF);
    buffer[(*index)++] = (uint8_t)((number >> 24) & 0xFF);
    return 0;
}

// Writes a 64-bit integer as 8 little-endian bytes
int write_uint64(uint8_t* buffer, uint32_t buffer_size, uint16_t* index, uint64_t number) {
    if (*index + 8 > buffer_size) {
        return -1;  // Buffer overflow
    }
    // Lower 32 bits
    buffer[(*index)++] = (uint8_t)(number & 0xFF);
    buffer[(*index)++] = (uint8_t)((number >> 8) & 0xFF);
    buffer[(*index)++] = (uint8_t)((number >> 16) & 0xFF);
    buffer[(*index)++] = (uint8_t)((number >> 24) & 0xFF);

    // Upper 32 bits
    buffer[(*index)++] = (uint8_t)((number >> 32) & 0xFF);
    buffer[(*index)++] = (uint8_t)((number >> 40) & 0xFF);
    buffer[(*index)++] = (uint8_t)((number >> 48) & 0xFF);
    buffer[(*index)++] = (uint8_t)((number >> 56) & 0xFF);
    return 0;
}

void create_test_gcda_file() {
    uint8_t buffer[256];
    uint16_t index = 0;
    uint32_t buffer_size = sizeof(buffer) / sizeof(buffer[0]);  // evaluates to 256

    // File Header
    write_uint32(buffer, buffer_size, &index, 0x67636461);  // Magic `gcda`
    write_uint32(buffer, buffer_size, &index, 0x4235322a);  // Version `B52*`
    write_uint32(buffer, buffer_size, &index, 2029845527);  // Stamp
    write_uint32(buffer, buffer_size, &index, 980088571);   // Checksum

    // OBJECT_SUMMARY Tag
    write_uint32(buffer, buffer_size, &index, 0xa1000000);  // Tag ID
    write_uint32(buffer, buffer_size, &index, 8);           // Length: 8 bytes
    write_uint32(buffer, buffer_size, &index, 3);           // runs
    write_uint32(buffer, buffer_size, &index, 30);          // sum_max

    // FUNCTION Tag
    write_uint32(buffer, buffer_size, &index, 0x01000000);  // Tag ID
    write_uint32(buffer, buffer_size, &index, 12);          // Length: 12 bytes
    write_uint32(buffer, buffer_size, &index, 108032747);   // ident
    write_uint32(buffer, buffer_size, &index, 0x392187d7);  // lineno_checksum
    write_uint32(buffer, buffer_size, &index, 0x758e20c5);  // cfg_checksum

    // COUNTERS arcs Tag
    write_uint32(buffer, buffer_size, &index, 0x01a10000);  // Tag ID
    write_uint32(buffer, buffer_size, &index, 64);          // Length: 64 bytes (8 counts * 8 bytes)

    // execution counts
    uint64_t counts[8] = {3, 3, 30, 6, 6, 0, 30, 30};
    for (int i = 0; i < 8; i++) {
        write_uint64(buffer, buffer_size, &index, counts[i]);
    }

    // EOF Tag
    write_uint32(buffer, buffer_size, &index, 0x00000000);  // EOF marker

    // Write buffer to file
    FILE* file = fopen("test.gcda", "wb");
    if (file) {
        // We write out `index` number of items, where each item is 1 byte long
        fwrite(buffer, 1, index, file);
        fclose(file);
        printf("Successfully created test.gcda!\n");
    } else {
        printf("Failed to create test.gcda.\n");
    }
}

int main(int argc, char* argv[]) {
    create_test_gcda_file();
    return 0;
}
