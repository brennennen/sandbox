
#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/emu_registers.h"
#include "libraries/emulate8086/include/decode_utils.h"
#include "libraries/emulate8086/include/decode_shared.h"

#include "libraries/emulate8086/include/instructions/logic/and.h"

// MARK: I_AND

emu_result_t decode_and(
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
        "and", 3, out_buffer, index, out_buffer_size
    );
    return result;
}

emu_result_t emu_and(emulator_t* emulator, uint8_t byte1) {
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

    printf("TODO: emu_and\n");
    return ER_FAILURE;
}

// MARK: I_AND_IMMEDIATE
emu_result_t decode_and_immediate(
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

    // NOTE: datasheet doesn't match current NASM implementation. NASM uses sign bit and this implementation matches.
    emu_result_t result = emu_decode_common_signed_immediate_format(
        emulator, byte1, &sign, &wide, &mod, &subcode, &rm, &displacement, &data, &instruction_size
    );

    write__common_immediate_to_register_or_memory(
        0, wide, mod, rm, displacement, data,
        "and", 3, out_buffer, index, out_buffer_size
    );

    return result;
}

emu_result_t emu_and_immediate(emulator_t* emulator, uint8_t byte1) {
    uint8_t sign = 0;
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t subcode = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint16_t data = 0;
    uint8_t instruction_size = 0;

    // NOTE: datasheet doesn't match current NASM implementation. NASM uses sign bit and this implementation matches.
    emu_result_t result = emu_decode_common_signed_immediate_format(
        emulator, byte1, &sign, &wide, &mod, &subcode, &rm, &displacement, &data, &instruction_size
    );

    printf("TODO: emu_and_immediate\n");
    return ER_FAILURE;
}

// MARK: I_AND_IMMEDIATE_TO_AX
emu_result_t decode_and_immediate_to_ax(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    uint8_t sign = (byte1 & 0b00000010) >> 1;
    wide_t wide = byte1 & 0b00000001;
    uint8_t instruction_size = 1;

    char* reg_string = "ax";
    uint16_t immediate = 0;
    if (wide == WIDE_BYTE) {
        reg_string = "al";
        emu_result_t read_result = dcd_read_byte(emulator, (uint8_t*) &immediate);
        if (read_result != ER_SUCCESS) {
            return ER_FAILURE;
        }
    } else {
        reg_string = "ax";
        emu_result_t read_result = dcd_read_word(emulator, &immediate);
        if (read_result != ER_SUCCESS) {
            return ER_FAILURE;
        }
    }

    int written = snprintf(out_buffer + *index,  out_buffer_size - *index, "and %s, %d",
        reg_string,
        immediate);
    if (written < 0) {
        // TODO: propogate error
        return ER_FAILURE;
    }
    *index += written;

    return ER_SUCCESS;
}

emu_result_t emu_and_immediate_to_ax(emulator_t* emulator, uint8_t byte1) {
    uint8_t sign = (byte1 & 0b00000010) >> 1;
    wide_t wide = byte1 & 0b00000001;
    uint8_t instruction_size = 1;

    printf("TODO: emu_and_immediate_to_ax\n");
    return ER_FAILURE;
}
