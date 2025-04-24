
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/registers.h"
#include "shared/include/result.h"

#include "libraries/emulate_intel/include/emulate.h"
#include "libraries/emulate_intel/include/emu_registers.h"
#include "libraries/emulate_intel/include/decode_utils.h"
#include "libraries/emulate_intel/include/decode_tag.h"
#include "libraries/emulate_intel/include/logger.h"

#include "libraries/emulate_intel/include/i386/decode_i386_common.h"

/**
 *
 */
emu_result_t emu_i386_read_displacement(
    emulator_t* emulator,
    mod_t mod,
    uint8_t rm_id,
    uint16_t* displacement,
    uint8_t* displacement_byte_size
) {
    if (mod == MOD_MEMORY) {
        if (rm_id == 0b00000110) {
            emu_result_t read_displace_result = dcd_read_word(emulator, displacement);
            *displacement_byte_size = 2;
            if (read_displace_result != ER_SUCCESS) {
                return read_displace_result;
            }
        }
    } else if (mod == MOD_MEMORY_8BIT_DISPLACEMENT) {
        emu_result_t read_displace_result = dcd_read_byte(emulator, (uint8_t*) displacement);
        *displacement_byte_size = 1;
        if (read_displace_result != ER_SUCCESS) {
            return read_displace_result;
        }
    } else if (mod == MOD_MEMORY_16BIT_DISPLACEMENT) {
        emu_result_t read_displace_result = dcd_read_word(emulator, displacement);
        *displacement_byte_size = 2;
        if (read_displace_result != ER_SUCCESS) {
            return read_displace_result;
        }
    } else {
        *displacement = 0;
    }
    return ER_SUCCESS;
}

emu_result_t emu_i386_decode_common_standard_format(
    emulator_t* emulator,
    uint8_t byte1,
    direction_t* direction,
    wide_t* wide,
    mod_t* mod,
    uint8_t* reg_id,
    uint8_t* rm_id,
    uint16_t* displacement,
    uint8_t* instruction_size
) {
    *instruction_size = 1;
    *direction = (byte1 & 0b00000010) >> 1;
    *wide = byte1 & 0b00000001;

    uint8_t byte2 = 0;
    emu_result_t read_byte2_result = dcd_read_byte(emulator, &byte2);
    if (read_byte2_result != ER_SUCCESS) {
        return read_byte2_result;
    }
    *instruction_size += 1;

    *mod = (byte2 & 0b11000000) >> 6;
    *reg_id = (byte2 & 0b00111000) >> 3;
    *rm_id = byte2 & 0b00000111;
    // TODO: handle displacement
    // uint8_t displacement_byte_size = 0;
    // emu_result_t read_displacement_result = read_displacement(emulator, *mod,
    //     *rm_id, displacement, &displacement_byte_size);
    // *instruction_size += displacement_byte_size;
    // if (read_displacement_result != ER_SUCCESS) {
    //     return read_displacement_result;
    // }

    return SUCCESS;
}

/**
 * (Intel 80386 Programmer's Reference Manual, 17.2.2.1, 246)
 */
static char i386_reg_m8_strings[][4] = {
    "al",
    "cl",
    "dl",
    "bl",
    "ah",
    "ch",
    "dh",
    "bh",
};

/**
 * (Intel 80386 Programmer's Reference Manual, 17.2.2.1, 246)
 */
static char i386_reg_m16_strings[][4] = {
    "ax",
    "cx",
    "dx",
    "bx",
    "sp",
    "bp",
    "si",
    "di",
};

/**
 * (Intel 80386 Programmer's Reference Manual, 17.2.2.1, 246)
 */
static char i386_reg_m32_strings[][4] = {
    "eax",
    "ecx",
    "edx",
    "ebx",
    "esp",
    "ebp",
    "esi",
    "edi",
};

static emu_result_t emu_i386_write_common_standard_format_register(
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
    int buffer_size
) {
    uint8_t* left = &rm;
    uint8_t* right = &reg;

    if (direction == DIR_REG_DEST) {
        left = &reg;
        right = &rm;
    }

    char* left_string = i386_reg_m16_strings[*left];
    char* right_string = i386_reg_m16_strings[*right];

    if (wide == WIDE_WORD) {
        left_string = i386_reg_m32_strings[*left];
        right_string = i386_reg_m32_strings[*right];
    }

    int written = snprintf(buffer + *index, buffer_size - *index, "%s %s, %s",
                            mnemonic,
                            left_string,
                            right_string);
    if (written < 0) {
        return(ER_FAILURE);
    }
    *index += written;
    return(ER_SUCCESS);
}

emu_result_t emu_i386_write_common_standard_format(
    direction_t direction,
    wide_t wide,
    mod_t mod,
    uint8_t reg_id,
    uint8_t rm_id,
    uint16_t displacement,
    char* mnemonic,
    uint8_t mnemonic_size,
    char* buffer,
    int* index,
    int buffer_size
) {
    if (mod == MOD_MEMORY && rm_id == REG_DIRECT_ACCESS) {
        //write__common_register_or_memory_with_register_or_memory__direct_access(direction, wide, mod, reg, rm, displacement, mnemonic, mnemonic_size, buffer, index, buffer_size);
        return(ER_FAILURE);
    }

    switch(mod) {
        case MOD_MEMORY:
        case MOD_MEMORY_8BIT_DISPLACEMENT:
        case MOD_MEMORY_16BIT_DISPLACEMENT: {
            //write__common_register_or_memory_with_register_or_memory__effective_address(direction, wide, mod, reg, rm, displacement, mnemonic, mnemonic_size, buffer, index, buffer_size);
            return(ER_FAILURE);
        }
        case MOD_REGISTER: {
            return(emu_i386_write_common_standard_format_register(direction, wide,
                mod, reg_id, rm_id, displacement, mnemonic, mnemonic_size, buffer,
                index, buffer_size));
        }
    }
    return(ER_FAILURE);
}

emu_result_t emu_i386_decode_and_write_common_standard_format(
    emulator_t* emulator,
    uint8_t byte1,
    char* mnemonic,
    uint8_t mnemonic_size,
    char* buffer,
    int* index,
    int buffer_size
) {
    direction_t direction = 0;
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t reg = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint8_t instruction_size = 0;

    emu_result_t result = emu_i386_decode_common_standard_format(
        emulator, byte1, &direction, &wide, &mod, &reg, &rm, &displacement, &instruction_size
    );
    if (result != ER_SUCCESS) {
        return(result);
    }

    result = emu_i386_write_common_standard_format(
        direction, wide, mod, reg, rm, displacement,
        mnemonic, mnemonic_size, buffer, index, buffer_size
    );
    if (result != ER_SUCCESS) {
        return(result);
    }

    return(ER_SUCCESS);
}
