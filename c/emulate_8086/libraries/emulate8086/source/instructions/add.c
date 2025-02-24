/**
 * `ADD destination, source`
 * The sum of the two operands, which may be bytes or words, replaces the destination
 * operand. Both operands may be signed or unsigned binary numbers (see AAA and DAA).
 * ADD updates AF, CF, OF, PF, SF, and ZF (8086 Family Users Manual, page 2-35,
 * pdf page ~50).
 *
 * This assembly instruction can be encoded into 3 different machine instructions.
 * The order of these encodings will follow the datasheet table 4-12 (8086 Family
 * Users Manual, page 4-23, pdf page ~165).
 *
 * ADD 1 - I_ADD
 * ADD 2 - I_ADD_IMMEDIATE
 * ADD 3 - I_ADD_IMMEDIATE_TO_AX
 */

#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/emu_registers.h"
#include "libraries/emulate8086/include/decode_utils.h"
#include "libraries/emulate8086/include/decode_shared.h"

#include "libraries/emulate8086/include/instructions/add.h"


void emu_internal_add_8bit(emulator_t* emulator, uint8_t* destination, uint8_t source) {
    uint8_t left = *destination;
    uint16_t uint16_result = (uint16_t)*destination + (uint16_t)source; // store the result in a larger result type to detect overflows.
    *destination = (uint8_t) uint16_result;
    emu_reg_update_auxiliary_carry_flag(&emulator->registers.flags, left, source, *destination);
    emu_reg_update_carry_flag_8bit(&emulator->registers.flags, uint16_result);
    emu_reg_update_overflow_flag_8bit(&emulator->registers.flags, left, source, *destination);
    emu_reg_update_parity_flag(&emulator->registers.flags, *destination);
    emu_reg_update_sign_flag_8bit(&emulator->registers.flags, *destination);
    emu_reg_update_zero_flag(&emulator->registers.flags, *destination);
}

void emu_internal_add_16bit(emulator_t* emulator, uint16_t* destination, uint16_t source) {
    uint16_t left = *destination;
    uint32_t uint32_result = (uint32_t)*destination + (uint32_t)source;
    *destination = (uint16_t) uint32_result;
    emu_reg_update_auxiliary_carry_flag(&emulator->registers.flags, left, source, *destination);
    emu_reg_update_carry_flag_16bit(&emulator->registers.flags, uint32_result);
    emu_reg_update_overflow_flag_16bit(&emulator->registers.flags, left, source, *destination);
    emu_reg_update_parity_flag(&emulator->registers.flags, *destination);
    emu_reg_update_sign_flag_16bit(&emulator->registers.flags, *destination);
    emu_reg_update_zero_flag(&emulator->registers.flags, *destination);
}

// MARK: ADD 1 - I_ADD
emu_result_t decode_add(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size)
{
    direction_t direction = 0;
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t reg = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint8_t instruction_size = 0;

    emu_result_t result = emu_decode_common_standard_format(
        emulator, byte1, &direction, &wide, &mod, &reg, &rm, &displacement, &instruction_size
    );

    write__common_register_or_memory_with_register_or_memory(
        direction, wide, mod, reg, rm, displacement,
        "add", 3, out_buffer, index, out_buffer_size
    );
    return result;
}

emu_result_t emu_add(emulator_t* emulator, uint8_t byte1) {
    direction_t direction = 0;
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t reg = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint8_t instruction_size = 0;

    emu_result_t result = emu_decode_common_standard_format(
        emulator, byte1, &direction, &wide, &mod, &reg, &rm, &displacement, &instruction_size
    );
    emulator->registers.ip += instruction_size;

    // TODO

    return ER_FAILURE;
}

// MARK: ADD 2 - I_ADD_IMMEDIATE

emu_result_t decode_add_immediate(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    uint8_t sign = 0;
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t subcode = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint16_t data = 0;
    uint8_t instruction_size = 0;

    emu_result_t result = emu_decode_common_immediate_format(
        emulator, byte1, &sign, &wide, &mod, &subcode, &rm, &displacement, &data, &instruction_size
    );

    write__common_immediate_to_register_or_memory(
        sign, wide, mod, rm, displacement, data,
        "add", 3, out_buffer, index, out_buffer_size
    );

    return result;
}

emu_result_t emu_add_immediate(emulator_t* emulator, uint8_t byte1) {
    uint8_t sign = 0;
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t subcode = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint16_t immediate = 0;
    uint8_t instruction_size = 0;

    emu_result_t result = emu_decode_common_immediate_format(
        emulator, byte1, &sign, &wide, &mod, &subcode, &rm, &displacement, &immediate, &instruction_size
    );
    emulator->registers.ip += instruction_size;

    if (result != ER_SUCCESS) {
        // todo: error out?
    }

    printf("sign: %d, wide: %d, mod: %d, rm: %d, displ: %d, immediate: %d\n",
        sign, wide, mod, rm, displacement, immediate);
    switch(mod) {
        case MOD_REGISTER: {
            if (wide == WIDE_BYTE) {
                uint8_t* destination = emu_get_byte_register(&emulator->registers, rm);
                emu_internal_add_8bit(emulator, destination, (uint8_t) immediate);
            } else {
                uint16_t* destination = emu_get_word_register(&emulator->registers, rm);
                emu_internal_add_16bit(emulator, destination, immediate);
            }
            return ER_SUCCESS;
            break;
        }
        default: {
            printf("emu_add_immediate: feature not implemented.");
            return ER_FAILURE;
            break;
        }
    }
    return ER_FAILURE;
}

// MARK: ADD 3 - I_ADD_IMMEDIATE_TO_AX
