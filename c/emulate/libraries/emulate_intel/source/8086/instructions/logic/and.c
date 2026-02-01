
#include <string.h>

#include "logger.h"
#include "shared/include/binary_utilities.h"
#include "shared/include/result.h"

#include "8086/decode_8086_shared.h"
#include "8086/decode_8086_utils.h"
#include "8086/emu_8086_registers.h"
#include "8086/emulate_8086.h"
#include "8086/instruction_tags_8086.h"

#include "8086/instructions/logic/and.h"

// MARK: I_AND
emu_result_t decode_and(
    emulator_8086_t* emulator,
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
        direction, wide, mod, reg, rm, displacement, "and", 3, out_buffer, index, out_buffer_size
    );
    return result;
}

/**
 *
 * Examples:
 * `and cl, [1000]`
 * `and [1000], bl`
 */
emu_result_t emu_and__direct_access(
    emulator_8086_t* emulator,
    direction_t direction,
    wide_t wide,
    uint8_t reg,
    uint8_t rm,
    uint16_t displacement
) {
    if (direction == DIR_REG_SOURCE) {
        if (wide == WIDE_BYTE) {
            uint8_t destination_value = 0;
            int res = emu_memory_get_byte(emulator, displacement, &destination_value);
            uint8_t* source = emu_get_byte_register(&emulator->registers, reg);
            emu_memory_set_byte(emulator, displacement, *source & destination_value);
            return (res);
        } else {  // WIDE_WORD
            uint16_t destination_value = 0;
            int res = emu_memory_get_uint16(emulator, displacement, &destination_value);
            uint16_t* source = emu_get_word_register(&emulator->registers, reg);
            emu_memory_set_uint16(emulator, displacement, *source & destination_value);
            return (res);
        }

    } else {  // DIR_REG_DEST
        if (wide == WIDE_BYTE) {
            uint8_t source_data = 0;
            int res = emu_memory_get_byte(emulator, displacement, &source_data);
            uint8_t* destination = emu_get_byte_register(&emulator->registers, reg);
            *destination = *destination & source_data;
            return (res);
        } else {  // WIDE_WORD
            uint16_t source_data = 0;
            int res = emu_memory_get_uint16(emulator, displacement, &source_data);
            uint16_t* destination = emu_get_word_register(&emulator->registers, reg);
            *destination = *destination & source_data;
            return (res);
        }
    }
}

/**
 *
 * Examples:
 * `and cl, [bx + 1000]`
 */
emu_result_t emu_and__effective_address(emulator_8086_t* emulator) {
    LOGD("TODO: emu_and effective address");

    return (ER_FAILURE);
}

/**
 *
 * Examples:
 * `and bl, cl`
 * `and cx, bx`
 */
emu_result_t emu_and__register(
    emulator_8086_t* emulator,
    direction_t direction,
    wide_t wide,
    uint8_t reg,
    uint8_t rm,
    mod_t mod,
    uint16_t displacement
) {
    uint8_t source_ref = reg;
    uint8_t destination_ref = rm;

    if (direction == DIR_REG_DEST) {
        source_ref = rm;
        destination_ref = reg;
    }
    if (wide == WIDE_BYTE) {
        uint8_t* source = emu_get_byte_register(&emulator->registers, source_ref);
        uint8_t* destination = emu_get_byte_register(&emulator->registers, destination_ref);
        *destination = *destination & *source;
    } else {
        uint16_t* source = emu_get_word_register(&emulator->registers, source_ref);
        uint16_t* destination = emu_get_word_register(&emulator->registers, destination_ref);
        *destination = *destination & *source;
    }
    return (ER_SUCCESS);
}

emu_result_t emu_and(emulator_8086_t* emulator, uint8_t byte1) {
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

    LOGDIW(
        write__common_register_or_memory_with_register_or_memory, direction, wide, mod, reg, rm,
        displacement, "and", 3
    );

    if (mod == MOD_MEMORY && rm == REG_DIRECT_ACCESS) {
        return (emu_and__direct_access(emulator, direction, wide, reg, rm, displacement));
    }

    switch (mod) {
        case MOD_MEMORY:
        case MOD_MEMORY_8BIT_DISPLACEMENT:
        case MOD_MEMORY_16BIT_DISPLACEMENT: {
            return (emu_and__effective_address(emulator));
        }
        case MOD_REGISTER: {
            return emu_and__register(emulator, direction, wide, reg, rm, mod, displacement);
        }
    }

    return (ER_FAILURE);
}

// MARK: I_AND_IMMEDIATE
emu_result_t decode_and_immediate(
    emulator_8086_t* emulator,
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

    // NOTE: datasheet doesn't match current NASM implementation. NASM uses sign bit and this
    // implementation matches.
    emu_result_t result = emu_decode_common_signed_immediate_format(
        emulator, byte1, &sign, &wide, &mod, &subcode, &rm, &displacement, &immediate,
        &instruction_size
    );

    write__common_immediate_to_register_or_memory(
        0, wide, mod, rm, displacement, immediate, "and", 3, out_buffer, index, out_buffer_size
    );

    return result;
}

static emu_result_t emu_add_immediate__direct_access(
    emulator_8086_t* emulator,
    wide_t wide,
    uint16_t displacement,
    uint16_t immediate
) {
    if (wide == WIDE_BYTE) {
        uint8_t source_data = 0;
        int res = emu_memory_get_byte(emulator, displacement, &source_data);
        if (res != ER_SUCCESS) {
            return res;
        }
        return emu_memory_set_byte(emulator, displacement, source_data & (uint8_t)immediate);
    } else {  // WIDE_WORD
        uint16_t source_data = 0;
        int res = emu_memory_get_uint16(emulator, displacement, &source_data);
        if (res != ER_SUCCESS) {
            return res;
        }
        return emu_memory_set_uint16(emulator, displacement, source_data & (uint8_t)immediate);
    }
}

static emu_result_t emu_add_immediate__effective_address(
    emulator_8086_t* emulator,
    wide_t wide,
    uint8_t rm,
    mod_t mod,
    uint16_t displacement,
    uint16_t immediate
) {
    uint32_t address = emu_get_effective_address(&emulator->registers, rm, mod, displacement);
    if (wide == WIDE_BYTE) {
        uint8_t source_data = 0;
        int res = emu_memory_get_byte(emulator, address, &source_data);
        if (res != ER_SUCCESS) {
            return res;
        }
        return emu_memory_set_byte(emulator, address, source_data & (uint8_t)immediate);
    } else {  // WIDE_WORD
        uint16_t source_data = 0;
        int res = emu_memory_get_uint16(emulator, address, &source_data);
        if (res != ER_SUCCESS) {
            return res;
        }
        return emu_memory_set_uint16(emulator, address, source_data & (uint8_t)immediate);
    }
}

static emu_result_t emu_add_immediate__register(
    emulator_8086_t* emulator,
    wide_t wide,
    uint8_t rm,
    uint16_t immediate
) {
    if (wide == WIDE_BYTE) {
        uint8_t* reg = emu_get_byte_register(&emulator->registers, rm);
        *reg = *reg & (uint8_t)immediate;
    } else {
        uint16_t* reg = emu_get_word_register(&emulator->registers, rm);
        *reg = *reg & immediate;
    }
    return (ER_SUCCESS);
}

emu_result_t emu_and_immediate(emulator_8086_t* emulator, uint8_t byte1) {
    uint8_t sign = 0;
    wide_t wide = 0;
    mod_t mod = 0;
    uint8_t subcode = 0;
    uint8_t rm = 0;
    uint16_t displacement = 0;
    uint16_t immediate = 0;
    uint8_t instruction_size = 0;

    // NOTE: datasheet doesn't match current NASM implementation. NASM uses sign bit and this
    // implementation matches.
    emu_result_t result = emu_decode_common_signed_immediate_format(
        emulator, byte1, &sign, &wide, &mod, &subcode, &rm, &displacement, &immediate,
        &instruction_size
    );
    if (result != ER_SUCCESS) {
        return (result);
    }

    LOGDIW(
        write__common_immediate_to_register_or_memory, 0, wide, mod, rm, displacement, immediate,
        "and", 3
    );

    if (mod == MOD_MEMORY && rm == REG_DIRECT_ACCESS) {
        return emu_add_immediate__direct_access(emulator, wide, displacement, immediate);
    }

    switch (mod) {
        case MOD_MEMORY:
        case MOD_MEMORY_8BIT_DISPLACEMENT:
        case MOD_MEMORY_16BIT_DISPLACEMENT: {
            return emu_add_immediate__effective_address(
                emulator, wide, rm, mod, displacement, immediate
            );
        }
        case MOD_REGISTER: {
            return emu_add_immediate__register(emulator, wide, rm, immediate);
        }
    }
    return (ER_FAILURE);
}

// MARK: I_AND_IMMEDIATE_TO_AX
void write_and_immediate_to_ax(
    wide_t wide,
    uint16_t immediate,
    char* buffer,
    int* index,
    int buffer_size
) {
    char* reg_string = "ax";
    if (wide == WIDE_BYTE) {
        reg_string = "al";
    } else {
        reg_string = "ax";
    }

    int written = snprintf(
        buffer + *index, buffer_size - *index, "and %s, %d", reg_string, immediate
    );
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
}

emu_result_t decode_and_immediate_to_ax(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
) {
    uint8_t sign = (byte1 & 0b00000010) >> 1;
    wide_t wide = byte1 & 0b00000001;

    uint16_t immediate = 0;
    if (wide == WIDE_BYTE) {
        emu_result_t read_result = dcd_read_byte(emulator, (uint8_t*)&immediate);
        if (read_result != ER_SUCCESS) {
            return (read_result);
        }
    } else {
        emu_result_t read_result = dcd_read_word(emulator, &immediate);
        if (read_result != ER_SUCCESS) {
            return (read_result);
        }
    }

    write_and_immediate_to_ax(wide, immediate, out_buffer, index, out_buffer_size);

    return (ER_SUCCESS);
}

emu_result_t emu_and_immediate_to_ax(emulator_8086_t* emulator, uint8_t byte1) {
    uint8_t sign = (byte1 & 0b00000010) >> 1;
    wide_t wide = byte1 & 0b00000001;

    if (wide == WIDE_BYTE) {
        uint8_t immediate = 0;
        emu_result_t read_result = dcd_read_byte(emulator, (uint8_t*)&immediate);
        if (read_result != ER_SUCCESS) {
            return (read_result);
        }
        emulator->registers.ax = (uint8_t)emulator->registers.ax & (uint8_t)immediate;
        LOGDIW(write_and_immediate_to_ax, wide, immediate);
    } else {  // WIDE_WORD
        uint16_t immediate = 0;
        emu_result_t read_result = dcd_read_word(emulator, &immediate);
        if (read_result != ER_SUCCESS) {
            return (read_result);
        }
        emulator->registers.ax = emulator->registers.ax & immediate;
        LOGDIW(write_and_immediate_to_ax, wide, immediate);
    }

    return (ER_SUCCESS);
}
