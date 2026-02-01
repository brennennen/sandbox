

#ifndef EMU_8086_CONDITIONAL_JUMPS_H
#define EMU_8086_CONDITIONAL_JUMPS_H

#include <stdint.h>

#include "8086/emulate_8086.h"
#include "8086/instruction_tags_8086.h"

// MARK: JE
emu_result_t decode_conditional_jump(
    emulator_8086_t* emulator,
    instruction_tag_8086_t tag,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);

emu_result_t emu_conditional_jump(
    emulator_8086_t* emulator,
    instruction_tag_8086_t tag,
    uint8_t byte1
);

emu_result_t emu_jne(emulator_8086_t* emulator, uint8_t byte1);

void write_conditional_jump(
    instruction_tag_8086_t tag,
    int8_t jump_offset,
    char* buffer,
    int* index,
    int buffer_size
);

#endif  // EMU_8086_CONDITIONAL_JUMPS_H
