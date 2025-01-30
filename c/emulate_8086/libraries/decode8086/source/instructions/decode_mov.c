/**
 * Responsible for decoding the "mov" assembly instruction for the 8086. This instruction
 * can be mapped into 7 different opcode encodings. The order of these encodings will
 * follow the datasheet table 4-12.
 *
 * 1. I_MOVE
 * 2. I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY
 * 3. I_MOVE_IMMEDIATE_TO_REGISTER
 * 4. I_MOVE_MEMORY_TO_ACCUMULATOR
 * 5. I_MOVE_ACCUMULATOR_TO_MEMORY
 * 6. I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER
 * 7. I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY
 *
 */

#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/decode8086/include/decode8086.h"
#include "libraries/decode8086/include/decode_utils.h"
#include "libraries/decode8086/include/decode_shared.h"

#include "libraries/decode8086/include/instructions/decode_mov.h"


// MARK: 1. I_MOVE

decode_result_t decode_move(
    decoder_t* decoder,
    uint8_t byte1,
    move_t* move)
{
    return decode__opcode_d_w__mod_reg_rm__disp_lo__disp_hi(
        decoder, byte1, &move->fields1, &move->fields2, &move->displacement
    );
}

void write_move(
    move_t* move,
    char* buffer,
    int* index,
    int buffer_size)
{
    wide_t wide = move->fields1 & 0b00000001;
    direction_t direction = (move->fields1 & 0b00000010) >> 1;
    mod_t mod = (move->fields2 & 0b11000000) >> 6;
    uint8_t reg = (move->fields2 & 0b00111000) >> 3;
    uint8_t rm = move->fields2 & 0b00000111;

    write__common_register_or_memory_with_register_or_memory(
        direction, wide, mod, reg, rm, move->displacement,
        "mov", 3, buffer, index, buffer_size);
}

// MARK: 2. I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY

decode_result_t decode__move_immediate_to_register_or_memory(
    decoder_t* decoder,
    uint8_t byte1,
    move_immediate_to_register_or_memory_t* move)
{
    move->fields1 = byte1;
    wide_t wide = move->fields1 & 0b00000001;

    if (dcd_read_byte(decoder, (uint8_t*) &move->fields2) == RI_FAILURE) {
        return DR_UNKNOWN_OPCODE;
    }
    mod_t mod = move->fields2 & 0b00000001;
    uint8_t rm = move->fields2 & 0b00000111;
    if (mod == MOD_MEMORY) {
        if (rm == 0b00000110) {
            decode_result_t read_displace_result = dcd_read_word(decoder, &move->displacement);
            if (read_displace_result != DR_SUCCESS) {
                return read_displace_result;
            }
        }
    } else if (mod == MOD_MEMORY_8BIT_DISPLACEMENT) {
        decode_result_t read_displace_result = dcd_read_byte(decoder, (uint8_t*) &move->displacement);
        if (read_displace_result != DR_SUCCESS) {
            return read_displace_result;
        }
    } else if (mod == MOD_MEMORY_16BIT_DISPLACEMENT) {
        decode_result_t read_displace_result = dcd_read_word(decoder, &move->displacement);
        if (read_displace_result != DR_SUCCESS) {
            return read_displace_result;
        }
    } else { // MOD_REGISTER
        // Don't have extra bytes for register to register movs. Nothing to do.
    }

    if (wide == WIDE_BYTE) {
        decode_result_t read_data_result = dcd_read_byte(decoder, (uint8_t*) &move->immediate);
    } else {
        decode_result_t read_data_result = dcd_read_word(decoder, &move->immediate);
    }

    return DR_SUCCESS;
}

void write__move_immediate_to_register_or_memory(
    move_immediate_to_register_or_memory_t* move,
    char* buffer,
    int* index,
    int buffer_size)
{
    wide_t wide = move->fields1 & 0b00000001;
    mod_t mod = (move->fields2 & 0b11000000) >> 6;
    uint8_t rm = move->fields2 & 0b00000111;

    char effective_address_string[32] = { 0 };
    build_effective_address(effective_address_string, sizeof(effective_address_string),
                            mod, rm, move->displacement);
    int written = snprintf(buffer + *index,  buffer_size - *index, "mov %s, %d",
                            effective_address_string,
                            move->immediate);
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
}

// MARK: 3. I_MOVE_IMMEDIATE_TO_REGISTER
decode_result_t decode__move_immediate_to_register(
    decoder_t* decoder,
    uint8_t byte1,
    move_immediate_to_register_t* move)
{
    move->fields1 = byte1;
    uint8_t wide = (move->fields1 & 0b00001000) >> 3;
    uint8_t reg = move->fields1 & 0b00000111;

    if (wide == WIDE_BYTE) {
        decode_result_t read_data_result = dcd_read_byte(decoder, (uint8_t*)&move->immediate);
        printf("immed to reg data: %d\n", move->immediate);
        if (read_data_result != DR_SUCCESS) {
            return FAILURE;
        }
    } else { // WIDE_WORD
        decode_result_t read_data_result = dcd_read_word(decoder, &move->immediate);
        printf("immed to reg (wide) data: %d\n", move->immediate);
        if (read_data_result != DR_SUCCESS) {
            return FAILURE;
        }
    }
    return DR_SUCCESS;
}

void write__move_immediate_to_register(
    move_immediate_to_register_t* move,
    char* buffer,
    int* index,
    int buffer_size)
{
    uint8_t wide = (move->fields1 & 0b00001000) >> 3;
    uint8_t reg = move->fields1 & 0b00000111;

    char* reg_string = regb_strings[reg];
    if (wide == WIDE_WORD) {
        reg_string = regw_strings[reg];
    }

    int written = snprintf(buffer + *index, buffer_size - *index, "mov %s, %d",
                            reg_string,
                            move->immediate);
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
}

// MARK: 4. I_MOVE_MEMORY_TO_ACCUMULATOR
decode_result_t decode__move_memory_to_accumulator(
    decoder_t* decoder,
    uint8_t byte1,
    move_memory_to_accumulator_t* move
) {
    move->fields1 = byte1;
    wide_t wide = move->fields1 & 0b00000001;
    decode_result_t read_data_result = dcd_read_word(decoder, &move->address);
    printf("address: %d\n", move->address);
    if (read_data_result != DR_SUCCESS) {
        return DR_FAILURE;
    }
    return DR_SUCCESS;
}

void write__move_memory_to_accumulator(
    move_memory_to_accumulator_t* move,
    char* buffer,
    int* index,
    int buffer_size
) {
    wide_t wide = move->fields1 & 0b00000001;
    char* reg_string = "al";
    if (wide == WIDE_WORD) {
        reg_string = "ax";
    }

    int written = snprintf(buffer + *index, buffer_size - *index, "mov %s, [%d]",
                            reg_string,
                            move->address);
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
}

// MARK: 5. I_MOVE_ACCUMULATOR_TO_MEMORY
decode_result_t decode__move_accumulator_to_memory(
    decoder_t* decoder,
    uint8_t byte1,
    move_accumulator_to_memory_t* move
) {
    move->fields1 = byte1;
    wide_t wide = move->fields1 & 0b00000001;
    decode_result_t read_data_result = dcd_read_word(decoder, &move->address);
    if (read_data_result != DR_SUCCESS) {
        return DR_FAILURE;
    }
    return DR_SUCCESS;
}

void write__move_accumulator_to_memory(
    move_accumulator_to_memory_t* move,
    char* buffer,
    int* index,
    int buffer_size
) {
    wide_t wide = move->fields1 & 0b00000001;
    char* reg_string = "al";
    if (wide == WIDE_WORD) {
        reg_string = "ax";
    }

    int written = snprintf(buffer + *index, buffer_size - *index, "mov [%d], %s",
                            move->address,
                            reg_string);
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
}