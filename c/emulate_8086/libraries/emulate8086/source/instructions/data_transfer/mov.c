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
 */

#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/decode_utils.h"
#include "libraries/emulate8086/include/decode_shared.h"
#include "libraries/emulate8086/include/logger.h"

#include "libraries/emulate8086/include/instructions/data_transfer/mov.h"


// MARK: 1. I_MOVE

emu_result_t decode_move(
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

    emu_result_t decode_result = emu_decode_common_standard_format(
        emulator, byte1, &direction, &wide, &mod, &reg, &rm, &displacement, &instruction_size
    );

    write__common_register_or_memory_with_register_or_memory(
        direction, wide, mod, reg, rm, displacement,
        "mov", 3, out_buffer, index, out_buffer_size
    );
    return decode_result;
}

/**
 * Performs a move between a register and a direct memory address.
 * Examples:
 * `mov [1000], bx`
 * `mov cx, [128]`
 */
static emu_result_t emu_move__direct_access(emulator_t* emulator, wide_t wide, uint8_t reg, uint16_t displacement) {
    if (wide == WIDE_BYTE) {
        uint8_t source_data = 0;
        int res = emu_memory_get_byte(emulator, displacement, &source_data);
        uint8_t* dest = emu_get_byte_register(&emulator->registers, reg);
        *dest = source_data;
        return res;
    } else { // WIDE_WORD
        uint16_t source_data = 0;
        int res = emu_memory_get_uint16(emulator, displacement, &source_data);
        uint16_t* dest = emu_get_word_register(&emulator->registers, reg);
        *dest = source_data;
        return res;
    }
}

/**
 * Performs a move between a register and an effective memory address.
 * Examples:
 * `mov [cx], bx`
 * `mov bx, [bx + si + 16]`
 */
static emu_result_t emu_move__memory(emulator_t* emulator, direction_t direction, wide_t wide, mod_t mode, uint8_t reg, uint8_t rm, uint16_t displacement) {
    uint32_t address = emu_get_effective_address(&emulator->registers, rm, mode, displacement);
    if (wide == WIDE_BYTE) {
        if (direction == DIR_REG_SOURCE) {
            uint8_t* source = emu_get_byte_register(&emulator->registers, reg);
            uint32_t dest_address = emu_get_effective_address(&emulator->registers, rm, mode, displacement);
            return emu_memory_set_byte(emulator, dest_address, *source);
        } else { // DIR_REG_DEST
            uint32_t source_address = emu_get_effective_address(&emulator->registers, rm, mode, displacement);
            uint8_t source = 0;
            result_t res = emu_memory_get_byte(emulator, source_address, &source);
            uint8_t* dest = emu_get_byte_register(&emulator->registers, reg);
            *dest = source;
            return res;
        }
    } else { // WIDE_WORD
        if (direction == DIR_REG_SOURCE) {
            uint16_t* source = emu_get_word_register(&emulator->registers, reg);
            uint32_t dest_address = emu_get_effective_address(&emulator->registers, rm, mode, displacement);
            return emu_memory_set_uint16(emulator, dest_address, *source);
        } else { // DIR_REG_DEST
            uint32_t source_address = emu_get_effective_address(&emulator->registers, rm, mode, displacement);
            uint16_t source = 0;
            result_t res = emu_memory_get_uint16(emulator, source_address, &source);
            uint16_t* dest = emu_get_word_register(&emulator->registers, reg);
            *dest = source;
            return res;
        }
    }
}

/**
 * Performs a move between 2 registers.
 * Examples:
 * `mov bx, cx`
 * `mov cx, dx`
 */
static emu_result_t emu_move__register(emulator_t* emulator, wide_t wide, uint8_t reg, uint8_t rm) {
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
}

/**
 * Implements the 8086 `mov` instruction first opcode (I_MOVE). Moves an 8 bit or 16
 * bit value between a source and destination (register to register, register to memory,
 * or memory to register).
 */
emu_result_t emu_move(emulator_t* emulator, uint8_t byte1) {
    direction_t direction = 0;
    wide_t wide = 0;
    mod_t mode = 0;
    uint8_t reg = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint8_t instruction_size = 0;

    emu_result_t decode_result = emu_decode_common_standard_format(
        emulator, byte1, &direction, &wide, &mode, &reg, &rm, &displacement, &instruction_size
    );

    LOGDIW(write__common_register_or_memory_with_register_or_memory, direction, wide, mode, reg, rm, displacement, "mov", 3);

    if (mode == MOD_MEMORY && rm == REG_DIRECT_ACCESS) {
        return emu_move__direct_access(emulator, wide, reg, displacement);
    }

    switch(mode) {
        case MOD_MEMORY:
        case MOD_MEMORY_8BIT_DISPLACEMENT:
        case MOD_MEMORY_16BIT_DISPLACEMENT: {
            return emu_move__memory(emulator, direction, wide, mode, reg, rm, displacement);
        }
        case MOD_REGISTER: {
            return emu_move__register(emulator, wide, reg, rm);
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

/**
 * Performs a move between an immediate and a direct memory address.
 * Examples:
 * `mov byte [1000], 42`
 */
emu_result_t emu_move_immediate__direct_access(emulator_t* emulator, wide_t wide,
    uint16_t displacement, uint16_t data)
{
    if (wide == WIDE_BYTE) {
        return emu_memory_set_byte(emulator, displacement, data);
    } else { // WIDE_WORD
        return emu_memory_set_uint16(emulator, displacement, data);
    }
}

/**
 * Implements the 8086 `mov` instruction first opcode (I_MOVE_IMMEDIATE). Moves
 * an 8 bit or 16 bit value between a source/destination and an immediate value
 * (immediate to register or immediate to memory).
 */
emu_result_t emu_move_immediate(emulator_t* emulator, uint8_t byte1) {
    wide_t wide = 0;
    mod_t mode = 0;
    uint8_t subcode = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint16_t data = 0;
    uint8_t instruction_size = 0;

    emu_result_t result = emu_decode_common_immediate_format(
        emulator, byte1, &wide, &mode, &subcode, &rm, &displacement, &data, &instruction_size
    );

    if (mode == MOD_MEMORY && rm == REG_DIRECT_ACCESS) {
        return emu_move_immediate__direct_access(emulator, wide, displacement, data);
    }

    switch(mode) {
        case MOD_MEMORY: {
            if (wide == WIDE_BYTE) {
                return emu_memory_set_byte(emulator, displacement, data);
            } else { // WIDE_WORD
                return emu_memory_set_uint16(emulator, displacement, data);
            }
        }
        case MOD_MEMORY_8BIT_DISPLACEMENT: {
            uint32_t effective_address = emu_get_effective_address(&emulator->registers,
                rm, mode, displacement);
            if (wide == WIDE_BYTE) {
                return emu_memory_set_byte(emulator, effective_address, data);
            } else { // WIDE_WORD
                return emu_memory_set_uint16(emulator, effective_address, data);
            }
        }
        case MOD_MEMORY_16BIT_DISPLACEMENT: {
            printf("emu_move_immediate: MOD_MEMORY_16BIT_DISPLACEMENT not implemented.\n");
            return ER_FAILURE;
        }
        default: {
            printf("emu_move_immediate: non-memory move not implemented.\n");
            return ER_FAILURE;
        }
    }
    return ER_FAILURE;
}

// MARK: 3. I_MOVE_IMMEDIATE_TO_REGISTER

emu_result_t read_move_immediate_to_register(
    emulator_t* emulator,
    uint8_t byte1,
    wide_t* wide,
    uint8_t* reg,
    uint16_t* immediate,
    uint8_t* instruction_size
) {
    *instruction_size = 1;
    *wide = (byte1 & 0b00001000) >> 3;
    *reg = byte1 & 0b00000111;
    if (*wide == WIDE_BYTE) {
        emu_result_t read_data_result = dcd_read_byte(emulator, (uint8_t*)immediate);
        *instruction_size += 1;
        if (read_data_result != ER_SUCCESS) {
            return FAILURE;
        }
    } else { // WIDE_WORD
        emu_result_t read_data_result = dcd_read_word(emulator, immediate);
        *instruction_size += 2;
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
    uint8_t instruction_size = 0;

    emu_result_t result = read_move_immediate_to_register(
        emulator, byte1, &wide, &reg, &immediate, &instruction_size);

    write_move_immediate_to_register(wide, reg, immediate, out_buffer, index, out_buffer_size);
    return result;
}

/**
 *
 */
emu_result_t emu_move_immediate_to_register(emulator_t* emulator, uint8_t byte1) {
    wide_t wide = 0;
    uint8_t reg = 0;
    uint16_t immediate = 0;
    uint8_t instruction_size = 0;

    emu_result_t result = read_move_immediate_to_register(
        emulator, byte1, &wide, &reg, &immediate, &instruction_size);
    if (wide == WIDE_BYTE) {
        uint8_t* left = emu_get_byte_register(&emulator->registers, reg);
        *left = immediate;
    } else {
        uint16_t* left = emu_get_word_register(&emulator->registers, reg);
        *left = immediate;
    }

    LOGDIW(write_move_immediate_to_register, wide, reg, immediate);

    return(ER_SUCCESS);
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

    LOGDIW(write_move_to_ax, wide, address);

    return(ER_FAILURE);
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

    LOGDIW(write_move_ax, wide, address);

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
}
