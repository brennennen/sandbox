

#ifndef DECODE_CONDITIONAL_JUMPS_H
#define DECODE_CONDITIONAL_JUMPS_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/decode8086.h"

// MARK: JE
emu_result_t decode_conditional_jump2(
    emulator_t* emulator,
    instruction_tag_t tag,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);

emu_result_t emu_conditional_jump(emulator_t* emulator, instruction_tag_t tag, uint8_t byte1);

void write_conditional_jump(
    instruction_tag_t tag,
    int8_t jump_offset,
    char* buffer,
    int* index,
    int buffer_size
);

#endif // DECODE_CONDITIONAL_JUMPS_H
