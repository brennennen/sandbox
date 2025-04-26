/**
 * Decodes machine code for 8086 assembly per the "8086 family users manual 1".
 */
#ifndef DECODE_i386_COMMON_H
#define DECODE_i386_COMMON_H

#include <stdio.h>

#include "shared/include/result.h"
#include "i386/instruction_tags_i386.h"
#include "i386/emulate_i386.h"

#include "8086/instruction_tags_8086.h"


#include "emulate.h"

emu_result_t emu_i386_read_displacement(
    emulator_i386_t* emulator,
    mod_t mod,
    uint8_t rm_id,
    uint16_t* displacement,
    uint8_t* displacement_byte_size
);

emu_result_t emu_i386_decode_common_standard_format(
    emulator_i386_t* emulator,
    uint8_t byte1,
    direction_t* direction,
    wide_t* wide,
    mod_t* mod,
    uint8_t* reg_id,
    uint8_t* rm_id,
    uint16_t* displacement,
    uint8_t* instruction_size
);

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
);

emu_result_t emu_i386_decode_and_write_common_standard_format(
    emulator_i386_t* emulator,
    uint8_t byte1,
    char* mnemonic,
    uint8_t mnemonic_size,
    char* buffer,
    int* index,
    int buffer_size
);

#endif // DECODE_i386_COMMON_H
