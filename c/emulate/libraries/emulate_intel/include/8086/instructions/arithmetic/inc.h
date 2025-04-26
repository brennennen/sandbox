/**
 * `INC destination`
 * INC (Increment) adds one to the destination operand. The operand may be a byte or a word
 * and is treated as an unsigned binary number (see AAA and DAA). INC updates AF, OF, PF, SF,
 * and ZF; it does not affect CF (8086 Family Users Manual, page 2-35, pdf page ~50).
 *
 * This assembly instruction can be encoded into 2 different machine instructions.
 * The order of these encodings will follow the datasheet table 4-12 (8086 Family
 * Users Manual, page 4-23, pdf page ~165).
 *
 * INC 1 - I_INC
 * INC 2 - I_INC_REGISTER
 */

#ifndef EMU_INC_H
#define EMU_INC_H

#include <stdint.h>

#include "8086/instruction_tags_8086.h"

#include "libraries/emulate_intel/include/emulate.h"

// MARK: INC 1 - I_INC
emu_result_t decode_inc(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_inc(emulator_8086_t* emulator, uint8_t byte1);

// INC 2 - I_INC_REGISTER
emu_result_t decode_inc_register(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_inc_register(emulator_8086_t* emulator, uint8_t byte1);

#endif // EMU_INC_H
