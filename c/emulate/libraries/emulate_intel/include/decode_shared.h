#ifndef DECODE_SHARED_H
#define DECODE_SHARED_H

#include <stdint.h>
#include <stdlib.h>

#include "shared/include/instructions.h"

#include "decode_utils.h"


static inline uint16_t emu_sign_extend_m8_to_m16(int8_t data) {
    // Performs a "signed extension" by taking advantage of c's 2 complement
    // implementation and casting.
    //
    // To save bytes, all 1 or all 0 leading immediates may truncate upper bytes.
    // That is, 0xFFF0 turns into 0xF0 in the machine code with the sign extension
    // and wide bits set. This (double) cast performs signed extension, so passing
    // in 0xF0 to this function turns it into 0xFFF0 (double cast assuming you pass
    // in a uint16_t, which is casted down to an int8_t).
    //
    // Example of what this function does:
    // Goal: turn 0xF0 back into 0xFFF0
    // uint16_t data = 0xF0 // 0xF0
    // int8_t temp = (int8_t) data; // 0xF0
    // uint16_t data = (uint16_t) temp; // 0xFFF0
    return (uint16_t)data;
}

emu_result_t read_displacement(
    emulator_t* emulator, mod_t mod,
    uint8_t rm, uint16_t* displacement,
    uint8_t* displacement_byte_size);

emu_result_t emu_decode_common_standard_format(
    emulator_t* emulator,
    uint8_t byte1,
    direction_t* direction,
    wide_t* wide,
    mod_t* mod,
    uint8_t* reg,
    uint8_t* rm,
    uint16_t* displacement,
    uint8_t* instruction_size
);

/**
 * Decodes a common 8086 4 - 6 byte variable structure that's shared across
 * many instructions.
 *
 * Machine code instructions that use this pattern:
 * MOV 2 - I_MOVE_IMMEDIATE
 * AND 2 - I_AND_IMMEDIATE
 * TEST 2 - I_TEST_IMMEDIATE
 * OR 2 - I_OR_IMMEDIATE
 * XOR 2 - I_XOR_IMMEDIATE
 *
 * See table 4-12 8086 Family Users Manual, page 4-22, pdf page ~164.
 *
 * byte1: [opcode wide]
 * byte2: [mod subcode r/m]
 * byte3: [displacement low]
 * byte4: [displacement high]
 * byte5: [data low]
 * byte6: [data high]
 */
emu_result_t emu_decode_common_immediate_format(
    emulator_t* emulator,
    uint8_t byte1,
    wide_t* wide,
    mod_t* mod,
    uint8_t* subcode,
    uint8_t* rm,
    uint16_t* displacement,
    uint16_t* data,
    uint8_t* instruction_size
);

/**
 * Decodes a common 8086 4 - 6 byte variable structure that's shared across
 * many instructions.
 *
 * Machine code instructions that use this pattern:
 * ADD 2 - I_ADD_IMMEDIATE
 * ADC 2 - I_ADC_IMMEDIATE
 * SUB 2 - I_SUB_IMMEDIATE
 * SBB 2 - I_SBB_IMMEDIATE
 * CMP 2 - I_CMP_IMMEDIATE
 *
 * See table 4-12 8086 Family Users Manual, page 4-22, pdf page ~164.
 *
 * byte1: [opcode sign wide]
 * byte2: [mod subcode r/m]
 * byte3: [displacement low]
 * byte4: [displacement high]
 * byte5: [data low]
 * byte6: [data high]
 */
emu_result_t emu_decode_common_signed_immediate_format(
    emulator_t* emulator,
    uint8_t byte1,
    uint8_t* sign,
    wide_t* wide,
    mod_t* mod,
    uint8_t* subcode,
    uint8_t* rm,
    uint16_t* displacement,
    uint16_t* data,
    uint8_t* instruction_size
);


void write_uint8(char* buffer, int* index, size_t buffer_size, uint8_t num);
void write_uint16(char* buffer, int* index, size_t buffer_size, uint16_t num);


char* map_register_field_encoding(reg_wide_t reg);

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

uint8_t* emu_get_byte_register(registers_t* registers, reg_t reg);
uint16_t* emu_get_word_register(registers_t* registers, reg_t reg);
uint32_t emu_get_effective_address(registers_t* registers, reg_t reg, mod_t mod, uint16_t displacement);
uint32_t emu_get_effective_address_mode_memory(registers_t* registers, reg_t reg);

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
