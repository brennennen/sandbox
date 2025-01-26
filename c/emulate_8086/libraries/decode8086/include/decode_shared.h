#ifndef DECODE_SHARED_H
#define DECODE_SHARED_H

#include <stdint.h>
#include <stdlib.h>

#include "shared/include/instructions.h"

void write_uint8(char* buffer, int* index, size_t buffer_size, uint8_t num);
void write_uint16(char* buffer, int* index, size_t buffer_size, uint16_t num);

void build_effective_address(char* buffer, size_t buffer_size,
                             mod_t mod, uint8_t rm, uint16_t displacement);

void write__common_register_or_memory_with_register_or_memory(
    direction_t direction,
    wide_t wide,
    mod_t mod,
    uint16_t reg,
    uint16_t rm,
    uint16_t displacement,
    char* mnemonic,
    uint8_t mnemonic_size,
    char* buffer,
    int* index,
    int buffer_size);

#endif // DECODE_SHARED_H
