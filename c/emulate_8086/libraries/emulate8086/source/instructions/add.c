/**
 * Responsible for decoding the "add" assembly instruction for the 8086. This assembly instruction
 * can be encoded into 3 different machine instructions. The order of these encodings will
 * follow the datasheet table 4-12.
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
#include "libraries/emulate8086/include/decode_utils.h"
#include "libraries/emulate8086/include/decode_shared.h"

#include "libraries/emulate8086/include/instructions/add.h"

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

    emu_result_t result = decode__opcode_d_w__mod_reg_rm__disp_lo__disp_hi(
        emulator, byte1, &direction, &wide, &mod, &reg, &rm, &displacement
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

    emu_result_t result = decode__opcode_d_w__mod_reg_rm__disp_lo__disp_hi(
        emulator, byte1, &direction, &wide, &mod, &reg, &rm, &displacement
    );

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

    emu_result_t result = decode__opcode_s_w__mod_subcode_rm__disp_lo__disp_hi__data_lo__data_hi(
        emulator, byte1, &sign, &wide, &mod, &subcode, &rm, &displacement, &data
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

    emu_result_t result = decode__opcode_s_w__mod_subcode_rm__disp_lo__disp_hi__data_lo__data_hi(
        emulator, byte1, &sign, &wide, &mod, &subcode, &rm, &displacement, &immediate
    );

    printf("sign: %d, wide: %d, mod: %d, rm: %d, displ: %d, immediate: %d\n",
        sign, wide, mod, rm, displacement, immediate);
    switch(mod) {
        case MOD_REGISTER: {
            if (wide == WIDE_BYTE) {
                uint8_t* left = emu_get_byte_register(&emulator->registers, rm);
                *left = *left + immediate;
                if (*left == 0) {
                    // TODO: set 0 flag?
                }
            } else {
                uint16_t* left = emu_get_word_register(&emulator->registers, rm);
                *left = *left + immediate;
                if (*left == 0) {
                    // TODO: set 0 flag?
                }
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
