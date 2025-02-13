#ifndef DECODE_SHARED_H
#define DECODE_SHARED_H

#include <stdint.h>
#include <stdlib.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/decode_utils.h"

emu_result_t read_displacement(emulator_t* emulator, mod_t mod, uint8_t rm, uint16_t* displacement);

emu_result_t decode__opcode_d_w__mod_reg_rm__disp_lo__disp_hi(
    emulator_t* emulator,
    uint8_t byte1,
    direction_t* direction,
    wide_t* wide,
    mod_t* mod,
    uint8_t* reg,
    uint8_t* rm,
    uint16_t* displacement
);

emu_result_t decode__opcode_s_w__mod_subcode_rm__disp_lo__disp_hi__data_lo__data_hi(
    emulator_t* emulator,
    uint8_t byte1,
    uint8_t* sign,
    wide_t* wide,
    mod_t* mod,
    uint8_t* subcode,
    uint8_t* rm,
    uint16_t* displacement,
    uint16_t* data
);


void write_uint8(char* buffer, int* index, size_t buffer_size, uint8_t num);
void write_uint16(char* buffer, int* index, size_t buffer_size, uint16_t num);

/**
 * Maps the R/M field encodings to a string format. This function can handle
 * all R/M field encodings, but should only be used for "Effective Address"
 * encodings. There are simpler solutions for the standard register encodings.
 * See "Table 4-10. R/M (Register/Memory) Field Encoding".
 *
 * @param buffer Where to write the string encoding of the effective address to.
 * @param buffer_size Size of the output buffer being written to.
 * @param wide Whether the R/M field represents a byte or word.
 * @param mod Mode field encoding, used to determine how to parse the R/M field.
 *              See tables 4-8 and 4-10. for more details.
 * @param rm Register/Memory field encoding being processed. See table 4-10.
 * @param displacement Offset to add to the address when mod is 0b01 or 0b10.
 */
void build_effective_address(char* buffer, size_t buffer_size,
                             wide_t wide, mod_t mod, uint8_t rm,
                             uint16_t displacement);

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
    uint8_t sign,
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
