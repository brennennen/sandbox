/**
 * x64: (not implemented)
 *
 * i386: (not implemented)
 *
 * 8086: (wip)
 * `AND destination,source`
 * AND performs the logical "and" of the two operands (byte or word) and returns
 * to the destination operand. A bit in the result is set if both corresponding
 * bits of the original operands are set; otherwise the bit is cleared (8086
 * Family Users Manual, page 2-38, pdf page ~53).
 *
 * AND (invert) has 3 machine instruction on 8086. See datasheet table 4-12
 * (8086 Family Users Manual, page 4-25, pdf page ~166).
 *
 * 1. I_AND
 * 2. I_AND_IMMEDIATE
 *    * NOTE: datasheet doesn't match NASM. 0x83 and 0x81 are seen, the sign bit
 *      was added in the second least significant bit of the first byte.
 * 3. I_AND_IMMEDIATE_TO_AX
 */
#ifndef EMU_8086_AND_H
#define EMU_8086_AND_H

#include <stdint.h>

#include "8086/emulate_8086.h"

// MARK: I_AND
emu_result_t decode_and(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_and(emulator_8086_t* emulator, uint8_t byte1);

// MARK: I_AND_IMMEDIATE
emu_result_t decode_and_immediate(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_and_immediate(emulator_8086_t* emulator, uint8_t byte1);

// MARK: I_AND_IMMEDIATE_TO_AX
emu_result_t decode_and_immediate_to_ax(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_and_immediate_to_ax(emulator_8086_t* emulator, uint8_t byte1);

#endif // EMU_8086_AND_H
