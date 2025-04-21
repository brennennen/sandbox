
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
    uint16_t immediate = 0;
    uint8_t instruction_size = 0;

    // NOTE: datasheet doesn't match current NASM implementation. NASM uses sign bit and this implementation matches.
    emu_result_t result = emu_decode_common_signed_immediate_format(
        emulator, byte1, &sign, &wide, &mod, &subcode, &rm, &displacement, &immediate, &instruction_size
    );

    write__common_immediate_to_register_or_memory(
        0, wide, mod, rm, displacement, immediate,
        "and", 3, out_buffer, index, out_buffer_size
    );

    return result;
}

static emu_result_t emu_add_immediate__register(emulator_t* emulator, wide_t wide, uint8_t rm, uint16_t immediate) {
    if (wide == WIDE_BYTE) {
        uint8_t* reg = emu_get_byte_register(&emulator->registers, rm);
        *reg = *reg & (uint8_t) immediate;
    } else {
        uint16_t* reg = emu_get_word_register(&emulator->registers, rm);
        *reg = *reg & immediate;
    }
    return(ER_SUCCESS);
}

emu_result_t emu_and_immediate(emulator_t* emulator, uint8_t byte1) {
    uint8_t sign = 0;
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t subcode = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint16_t immediate = 0;
    uint8_t instruction_size = 0;

    // NOTE: datasheet doesn't match current NASM implementation. NASM uses sign bit and this implementation matches.
    emu_result_t result = emu_decode_common_signed_immediate_format(
        emulator, byte1, &sign, &wide, &mod, &subcode, &rm, &displacement, &immediate, &instruction_size
    );
    if (result != ER_SUCCESS) {
        return(result);
    }

    if (mod == MOD_MEMORY && rm == REG_DIRECT_ACCESS) {
        printf("todo - emu_and_immediate direct access\n");
    }

    switch(mod) {
        case MOD_MEMORY:
        case MOD_MEMORY_8BIT_DISPLACEMENT:
        case MOD_MEMORY_16BIT_DISPLACEMENT: {
            printf("todo - emu_and_immediate memory\n");
            break;
        }
        case MOD_REGISTER: {
            emu_add_immediate__register(emulator, wide, rm, immediate);
            result = ER_SUCCESS;
            break;
        }
    }

    return(result);
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
            return(read_result);
        }
    } else {
        reg_string = "ax";
        emu_result_t read_result = dcd_read_word(emulator, &immediate);
        if (read_result != ER_SUCCESS) {
            return(read_result);
        }
    }

    int written = snprintf(out_buffer + *index,  out_buffer_size - *index, "and %s, %d",
        reg_string,
        immediate);
    if (written < 0) {
        // TODO: propogate error
        return(ER_FAILURE);
    }
    *index += written;

    return(ER_SUCCESS);
}

emu_result_t emu_and_immediate_to_ax(emulator_t* emulator, uint8_t byte1) {
    uint8_t sign = (byte1 & 0b00000010) >> 1;
    wide_t wide = byte1 & 0b00000001;
    uint8_t instruction_size = 1;

    printf("TODO: emu_and_immediate_to_ax\n");
    return(ER_FAILURE);
}
