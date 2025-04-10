/**
 * `XCHG destination,source`
 * XCHG (exchange) switches the contents of the source and destination (byte or word)
 * operands. When used in conjunction with the LOCK prefix, XCHG can test and set a
 * semaphore that controls access toa resource shared by multiple processors.
 *
 * This instruction can be mapped into 2 different opcode encodings. The order of these
 * encodings will follow the datasheet table 4-12 (8086 Family Users Manual, page 4-22,
 * pdf page ~164).
 *
 * 1. I_EXCHANGE
 * 2. I_EXCHANGE_AX
 *
 */

#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/emu_registers.h"
#include "libraries/emulate8086/include/decode_utils.h"
#include "libraries/emulate8086/include/decode_shared.h"

#include "libraries/emulate8086/include/instructions/data_transfer/xchg.h"

// MARK: 1. I_EXCHANGE
emu_result_t read_exchange(
    emulator_t* emulator,
    uint8_t byte1,
    wide_t* wide,
    mod_t* mod,
    uint8_t* reg,
    uint8_t* rm,
    uint16_t* displacement,
    uint8_t* instruction_size
) {
    *instruction_size = 1;
    *wide = byte1 & 0b00000001;
    uint8_t byte2 = 0;
    if (dcd_read_byte(emulator, (uint8_t*) &byte2) == RI_FAILURE) {
        return ER_UNKNOWN_OPCODE;
    }

    *mod = (byte2 & 0b11000000) >> 6;
    *reg = (byte2 & 0b00111000) >> 3;
    *rm = byte2 & 0b00000111;
    // TODO: read displacement if mod is a certain value:
    // if (*wide == WIDE_BYTE) {
    //     emu_result_t read_data_result = dcd_read_byte(emulator, (uint8_t*)displacement);
    //     *instruction_size += 1;
    //     if (read_data_result != ER_SUCCESS) {
    //         return FAILURE;
    //     }
    // } else { // WIDE_WORD
    //     emu_result_t read_data_result = dcd_read_word(emulator, displacement);
    //     *instruction_size += 2;
    //     if (read_data_result != ER_SUCCESS) {
    //         return FAILURE;
    //     }
    // }
    return ER_SUCCESS;
}
emu_result_t decode_exchange(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    direction_t direction = 0; // not used
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t reg = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint8_t instruction_size = 0;

    emu_result_t result = read_exchange(
        emulator, byte1, &wide, &mod, &reg, &rm, &displacement, &instruction_size);

    write__common_register_or_memory_with_register_or_memory(
        direction, wide, mod, reg, rm, displacement,
        "xchg", 4, out_buffer, index, out_buffer_size
    );
    return result;
}

emu_result_t emu_exchange(emulator_t* emulator, uint8_t byte1) {
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t reg = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint8_t instruction_size = 0;

    emu_result_t result = read_exchange(
        emulator, byte1, &wide, &mod, &reg, &rm, &displacement, &instruction_size);

    emulator->registers.ip += instruction_size;

    // TODO

    return ER_FAILURE;
}


// MARK: 2. I_EXCHANGE_AX
emu_result_t decode_exchange_ax(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    uint8_t reg = byte1 & 0b00000111;
    char* reg_string = map_register_field_encoding(reg);
    int written = snprintf(out_buffer + *index,  out_buffer_size - *index, "xchg %s, ax\n", reg_string);
    if (written < 0) {
        return ER_FAILURE;
    }
    *index += written;
    return ER_SUCCESS;
}

emu_result_t emu_exchange_ax(emulator_t* emulator, uint8_t byte1) {
    uint8_t reg = byte1 & 0b00000111;

    emulator->registers.ip += 1;

    // TODO

    return ER_FAILURE;
}

// MARK: 3. I_PUSH_SEGMENT_REGISTER

