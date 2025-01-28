#ifndef DECODE_SHARED_H
#define DECODE_SHARED_H

#include <stdint.h>
#include <stdlib.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode_utils.h"

decode_result_t read_displacement(decoder_t* decoder, mod_t mod, uint8_t rm, uint16_t* displacement);

decode_result_t decode__opcode_d_w__mod_reg_rm__disp_lo__disp_hi(
    decoder_t* decoder,
    uint8_t byte1,
    uint8_t* fields1,
    uint8_t* fields2,
    uint16_t* displacement
);

void write_uint8(char* buffer, int* index, size_t buffer_size, uint8_t num);
void write_uint16(char* buffer, int* index, size_t buffer_size, uint16_t num);

void build_effective_address(char* buffer, size_t buffer_size,
                             mod_t mod, uint8_t rm, uint16_t displacement);

void write__common_register_or_memory_with_register_or_memory(
    direction_t direction,
    wide_t wide,
    mod_t mod,
    uint8_t reg,
    uint8_t rm,
    uint16_t displacement,
    char* mnemonic,
    uint8_t mnemonic_size,
    char* buffer,
    int* index,
    int buffer_size);

void write__common_immediate_to_register_or_memory(
    direction_t direction,
    wide_t wide,
    mod_t mod,
    uint8_t rm,
    uint16_t displacement,
    uint16_t immediate,
    char* mnemonic,
    uint8_t mnemonic_size,
    char* buffer,
    int* index,
    int buffer_size);



#endif // DECODE_SHARED_H
