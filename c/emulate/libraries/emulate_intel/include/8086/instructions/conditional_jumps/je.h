
#ifndef I_JE_H
#define I_JE_H

#include <stdint.h>

#include "8086/instruction_tags_8086.h"

#include "libraries/emulate_intel/include/emulate.h"

// MARK: JE/JZ 1 - I_JUMP_ON_EQUAL
// emu_result_t decode_jump_on_equal(
//     emulator_8086_t* emulator,
//     uint8_t byte1,
//     jump_on_equal_t* jump_on_equal
// );
// void write_jump_on_equal(
//     jump_on_equal_t* jump_on_equal,
//     char* buffer,
//     int* index,
//     int buffer_size
// );

#endif // I_JE_H
