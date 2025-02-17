/**
 * `MOV destination, source`
 * MOV transfers a byte or a word from the source operand to the destination operand.
 *
 * One of the 14 data transfer instructions. Categorized as a "General Purpose" data
 * transfer instruction (see Table 2-8. Data Transfer Instructions).
 *
 * This instruction can be mapped into 7 different opcode encodings. The order of
 * these encodings will follow the datasheet table 4-12.
 *
 * 1. I_MOVE
 * 2. I_MOVE_IMMEDIATE
 * 3. I_MOVE_IMMEDIATE_TO_REGISTER
 * 4. I_MOVE_TO_AX
 * 5. I_MOVE_AX
 * 6. I_MOVE_TO_SEGMENT_REGISTER
 * 7. I_MOVE_SEGMENT_REGISTER
 *
 */

#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/decode_utils.h"
#include "libraries/emulate8086/include/decode_shared.h"

#include "libraries/emulate8086/include/instructions/mov.h"


// MARK: 1. I_MOVE

emu_result_t decode_move(
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

    emu_result_t decode_result = emu_decode_common_standard_format(
        emulator, byte1, &direction, &wide, &mod, &reg, &rm, &displacement
    );
    write__common_register_or_memory_with_register_or_memory(
        direction, wide, mod, reg, rm, displacement,
        "mov", 3, out_buffer, index, out_buffer_size
    );
    return decode_result;
}

emu_result_t emu_move(emulator_t* emulator, uint8_t byte1) {
    direction_t direction = 0;
    wide_t wide = 0;
    mod_t mode = 0;
    uint8_t reg = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;

    emu_result_t decode_result = emu_decode_common_standard_format(
        emulator, byte1, &direction, &wide, &mode, &reg, &rm, &displacement
    );

    // TODO
    printf("dir: %d, wide: %d, mode: %d, reg: %d, rm: %d, disp: %d\n",
        direction, wide, mode, reg, rm, displacement);
    switch(mode) {
        case MOD_REGISTER: {
            if (wide == WIDE_BYTE) {
                uint8_t* left = emu_get_byte_register(&emulator->registers, rm);
                uint8_t* right = emu_get_byte_register(&emulator->registers, reg);
                *left = *right;
            } else {
                uint16_t* left = emu_get_word_register(&emulator->registers, rm);
                uint16_t* right = emu_get_word_register(&emulator->registers, reg);
                *left = *right;
            }
            return ER_SUCCESS;
            break;
        }
        default: {
            printf("emu_move with non-register movs: not implemented.");
            return ER_FAILURE;
            break;
        }
    }
    return ER_FAILURE;
}

// MARK: 2. I_MOVE_IMMEDIATE
emu_result_t read_move_immediate(
    emulator_t* emulator,
    uint8_t byte1,
    wide_t* wide,
    mod_t* mod,
    uint8_t* subcode,
    uint8_t* rm,
    uint16_t* displacement,
    uint16_t* data
) {
    *wide = byte1 & 0b00000001;
    uint8_t byte2 = 0;
    if (dcd_read_byte(emulator, (uint8_t*) &byte2) == RI_FAILURE) {
        return ER_UNKNOWN_OPCODE;
    }
    *mod = (byte2 & 0b11000000) >> 6;
    *subcode = (byte2 & 0b00111000) >> 3;
    *rm = byte2 & 0b00000111;
    if (*mod == MOD_MEMORY) {
        if (*rm == 0b00000110) {
            emu_result_t read_displace_result = dcd_read_word(emulator, displacement);
            if (read_displace_result != ER_SUCCESS) {
                return read_displace_result;
            }
        }
    } else if (*mod == MOD_MEMORY_8BIT_DISPLACEMENT) {
        emu_result_t read_displace_result = dcd_read_byte(emulator, (uint8_t*) displacement);
        if (read_displace_result != ER_SUCCESS) {
            return read_displace_result;
        }
    } else if (*mod == MOD_MEMORY_16BIT_DISPLACEMENT) {
        emu_result_t read_displace_result = dcd_read_word(emulator, displacement);
        if (read_displace_result != ER_SUCCESS) {
            return read_displace_result;
        }
    } else { // MOD_REGISTER
        // Don't have extra bytes for register to register movs. Nothing to do.
    }

    if (*wide == WIDE_BYTE) {
        emu_result_t read_data_result = dcd_read_byte(emulator, (uint8_t*) data);
    } else {
        emu_result_t read_data_result = dcd_read_word(emulator, data);
    }

    return ER_SUCCESS;
}

emu_result_t decode_move_immediate(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size)
{
    uint8_t sign = 0;
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t subcode = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint16_t data = 0;

    emu_result_t result = read_move_immediate(
        emulator, byte1, &wide, &mod, &subcode, &rm, &displacement, &data
    );

    write__common_immediate_to_register_or_memory(
        sign, wide, mod, rm, displacement, data,
        "mov", 3, out_buffer, index, out_buffer_size
    );

    return result;
}

emu_result_t emu_move_immediate(emulator_t* emulator, uint8_t byte1) {
    uint8_t sign = 0;
    wide_t wide = 0;
    mod_t mode = 0;
    uint8_t subcode = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint16_t data = 0;
    emu_result_t result = emu_decode_common_immediate_format(
        emulator, byte1, &sign, &wide, &mode, &subcode, &rm, &displacement, &data
    );

    // TODO

    return ER_FAILURE;
}

// MARK: 3. I_MOVE_IMMEDIATE_TO_REGISTER

emu_result_t read_move_immediate_to_register(
    emulator_t* emulator,
    uint8_t byte1,
    wide_t* wide,
    uint8_t* reg,
    uint16_t* immediate
) {
    *wide = (byte1 & 0b00001000) >> 3;
    *reg = byte1 & 0b00000111;
    if (*wide == WIDE_BYTE) {
        emu_result_t read_data_result = dcd_read_byte(emulator, (uint8_t*)immediate);
        printf("immed to reg data: %d\n", *immediate);
        if (read_data_result != ER_SUCCESS) {
            return FAILURE;
        }
    } else { // WIDE_WORD
        emu_result_t read_data_result = dcd_read_word(emulator, immediate);
        printf("immed to reg (wide) data: %d\n", *immediate);
        if (read_data_result != ER_SUCCESS) {
            return FAILURE;
        }
    }
    return ER_SUCCESS;
}
emu_result_t decode_move_immediate_to_register(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size)
{
    wide_t wide = 0;
    uint8_t reg = 0;
    uint16_t immediate = 0;

    emu_result_t result = read_move_immediate_to_register(
        emulator, byte1, &wide, &reg, &immediate);

    write_move_immediate_to_register(wide, reg, immediate, out_buffer, index, out_buffer_size);
    return result;
}

emu_result_t emu_move_immediate_to_register(emulator_t* emulator, uint8_t byte1) {
    wide_t wide = 0;
    uint8_t reg = 0;
    uint16_t immediate = 0;

    emu_result_t result = read_move_immediate_to_register(
        emulator, byte1, &wide, &reg, &immediate);

    printf("wide: %d, reg: %d, immediate: %d\n",
        wide, reg, immediate);
    if (wide == WIDE_BYTE) {
        uint8_t* left = emu_get_byte_register(&emulator->registers, reg);
        *left = immediate;
    } else {
        uint16_t* left = emu_get_word_register(&emulator->registers, reg);
        *left = immediate;
    }
    return ER_SUCCESS;
}

void write_move_immediate_to_register(
    wide_t wide,
    uint8_t reg,
    uint16_t immediate,
    char* buffer,
    int* index,
    int buffer_size)
{
    char* reg_string = regb_strings[reg];
    if (wide == WIDE_WORD) {
        reg_string = regw_strings[reg];
    }

    int written = snprintf(buffer + *index, buffer_size - *index, "mov %s, %d",
                            reg_string,
                            immediate);
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
    snprintf(buffer + *index, buffer_size - *index, "\n");
    *index += 1;
}

// MARK: 4. I_MOVE_TO_AX
emu_result_t read_move_to_ax(
    emulator_t* emulator,
    uint8_t byte1,
    wide_t* wide,
    uint16_t* address
) {
   *wide = byte1 & 0b00000001;
    emu_result_t read_data_result = dcd_read_word(emulator, address);
    printf("address: %d\n", *address);
    if (read_data_result != ER_SUCCESS) {
        return ER_FAILURE;
    }
    return ER_SUCCESS;
}

emu_result_t decode_move_to_ax(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    wide_t wide = 0;
    uint16_t address = 0;
    emu_result_t read_data_result = read_move_to_ax(emulator, byte1, &wide, &address);
    write_move_to_ax(wide, address, out_buffer, index, out_buffer_size);
    return read_data_result;
}

emu_result_t emu_move_to_ax(emulator_t* emulator, uint8_t byte1) {
    wide_t wide = 0;
    uint16_t address = 0;

    emu_result_t read_data_result = read_move_to_ax(emulator, byte1, &wide, &address);

    // TODO

    return ER_FAILURE;
}

void write_move_to_ax(
    wide_t wide,
    uint16_t address,
    char* buffer,
    int* index,
    int buffer_size
) {
    char* reg_string = "al";
    if (wide == WIDE_WORD) {
        reg_string = "ax";
    }

    int written = snprintf(buffer + *index, buffer_size - *index, "mov %s, [%d]",
                            reg_string,
                            address);
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
    snprintf(buffer + *index, buffer_size - *index, "\n");
    *index += 1;
}

// MARK: 5. I_MOVE_AX
emu_result_t read_move_ax(
    emulator_t* emulator,
    uint8_t byte1,
    wide_t* wide,
    uint16_t* address
) {
   *wide = byte1 & 0b00000001;
    emu_result_t read_data_result = dcd_read_word(emulator, address);
    printf("address: %d\n", *address);
    if (read_data_result != ER_SUCCESS) {
        return ER_FAILURE;
    }
    return ER_SUCCESS;
}

emu_result_t decode_move_ax(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    wide_t wide = 0;
    uint16_t address = 0;
    emu_result_t result = read_move_ax(emulator, byte1, &wide, &address);
    write_move_ax(wide, address, out_buffer, index, out_buffer_size);
    return result;
}

emu_result_t emu_move_ax(emulator_t* emulator, uint8_t byte1) {
    wide_t wide = 0;
    uint16_t address = 0;
    emu_result_t result = read_move_ax(emulator, byte1, &wide, &address);

    // TODO

    return ER_FAILURE;
}

void write_move_ax(
    wide_t wide,
    uint16_t address,
    char* buffer,
    int* index,
    int buffer_size
) {
    char* reg_string = "al";
    if (wide == WIDE_WORD) {
        reg_string = "ax";
    }

    int written = snprintf(buffer + *index, buffer_size - *index, "mov [%d], %s",
                            address,
                            reg_string);
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
    snprintf(buffer + *index, buffer_size - *index, "\n");
    *index += 1;
}
