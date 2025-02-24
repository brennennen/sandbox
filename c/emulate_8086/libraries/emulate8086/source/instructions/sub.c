




#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/emu_registers.h"
#include "libraries/emulate8086/include/decode_utils.h"
#include "libraries/emulate8086/include/decode_shared.h"

#include "libraries/emulate8086/include/instructions/sub.h"

void emu_internal_sub_8bit(emulator_t* emulator, uint8_t* destination, uint8_t source) {
    uint8_t left = *destination;
    uint16_t uint16_result = (uint16_t)*destination - (uint16_t)source; // store the result in a larger result type to detect overflows.
    *destination = (uint8_t) uint16_result;
    emu_reg_update_auxiliary_carry_flag(&emulator->registers.flags, left, source, *destination);
    emu_reg_update_carry_flag_8bit(&emulator->registers.flags, uint16_result);
    emu_reg_update_overflow_flag_8bit(&emulator->registers.flags, left, source, *destination);
    emu_reg_update_parity_flag(&emulator->registers.flags, *destination);
    emu_reg_update_sign_flag_8bit(&emulator->registers.flags, *destination);
    emu_reg_update_zero_flag(&emulator->registers.flags, *destination);
}

void emu_internal_sub_16bit(emulator_t* emulator, uint16_t* destination, uint16_t source) {
    uint16_t left = *destination;
    uint32_t uint32_result = (uint32_t)*destination - (uint32_t)source;
    *destination = (uint16_t) uint32_result;
    emu_reg_update_auxiliary_carry_flag(&emulator->registers.flags, left, source, *destination);
    emu_reg_update_carry_flag_16bit(&emulator->registers.flags, uint32_result);
    emu_reg_update_overflow_flag_16bit(&emulator->registers.flags, left, source, *destination);
    emu_reg_update_parity_flag(&emulator->registers.flags, *destination);
    emu_reg_update_sign_flag_16bit(&emulator->registers.flags, *destination);
    emu_reg_update_zero_flag(&emulator->registers.flags, *destination);
}


// MARK: SUB 1 - I_SUB
emu_result_t decode_sub(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
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
        "sub", 3, out_buffer, index, out_buffer_size
    );
    return result;
}

emu_result_t emu_sub(emulator_t* emulator, uint8_t byte1) {
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

// MARK: SUB 2 - I_SUB_IMMEDIATE
emu_result_t decode_sub_immediate(
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
        "sub", 3, out_buffer, index, out_buffer_size
    );

    return result;
}

emu_result_t emu_sub_immediate(emulator_t* emulator, uint8_t byte1) {
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

    printf("sign: %d, wide: %d, mod: %d, rm: %d, displ: %d, immediate: %d\n",
        sign, wide, mod, rm, displacement, immediate);
    switch(mod) {
        case MOD_REGISTER: {
            if (wide == WIDE_BYTE) {
                uint8_t* destination = emu_get_byte_register(&emulator->registers, rm);
                emu_internal_sub_8bit(emulator, destination, (uint8_t) immediate);
            } else {
                uint16_t* destination = emu_get_word_register(&emulator->registers, rm);
                emu_internal_sub_16bit(emulator, destination, immediate);
            }
            return ER_SUCCESS;
            break;
        }
        default: {
            printf("emu_sub_immediate: feature not implemented.");
            return ER_FAILURE;
            break;
        }
    }

    return result;
}

// MARK: SUB 3 - I_SUB_IMMEDIATE_TO_AX

