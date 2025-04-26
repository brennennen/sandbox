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

#ifndef DECODE_ADD_H
#define DECODE_ADD_H

#include <stdint.h>

#include "8086/instruction_tags_8086.h"

#include "libraries/emulate_intel/include/emulate.h"

// MARK: ADD 1 - I_ADD
emu_result_t decode_add(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_add(emulator_8086_t* emulator, uint8_t byte1);

// MARK: ADD 2 - I_ADD_IMMEDIATE
emu_result_t decode_add_immediate(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_add_immediate(emulator_8086_t* emulator, uint8_t byte1);

// MARK: ADD 3 - I_ADD_IMMEDIATE_TO_AX


#endif
