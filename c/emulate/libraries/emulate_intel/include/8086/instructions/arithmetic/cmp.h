/**
 * `CMP destination, source`
 * CMP (Compare) subtracts the source from the destination, which may be bytes or
 * words, but does not return the result. The operands are unchanged but the flags
 * are updated and can be tested by a subsequent conditional jump instruction.
 * CMP updates AF, CF, OF, PF, SF, and ZF. The comparison reflected in the flags
 * is that of the destination to the source. If a CMP instruction is followed by
 * a JG (jump if greater) instruction, for example, the jump is taken if the
 * destination operand is greater than the source operand (8086 Family Users Manual,
 * page 2-36, pdf page ~51).
 *
 * This assembly instruction can be encoded into 3 different machine instructions.
 * The order of these encodings will follow the datasheet table 4-12 (8086 Family
 * Users Manual, page 4-23, pdf page ~165).
 *
 * CMP 1 - I_CMP
 * CMP 2 - I_COMPARE_IMMEDIATE
 * CMP 3 - I_COMPARE_IMMEDIATE_TO_AX
 */

#ifndef DECODE_CMP_H
#define DECODE_CMP_H

#include <stdint.h>

#include "8086/instruction_tags_8086.h"

#include "libraries/emulate_intel/include/emulate.h"

// MARK: CMP 1 - I_CMP
emu_result_t decode_compare(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_compare(emulator_8086_t* emulator, uint8_t byte1);

// MARK: CMP 2 - I_COMPARE_IMMEDIATE
emu_result_t decode_cmp_immediate(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_cmp_immediate(emulator_8086_t* emulator, uint8_t byte1);

// MARK: CMP 3 - I_COMPARE_IMMEDIATE_TO_AX


#endif // DECODE_CMP_H
