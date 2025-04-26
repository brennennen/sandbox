

#ifndef DECODE_SUB_H
#define DECODE_SUB_H

#include <stdint.h>

#include "8086/instruction_tags_8086.h"

#include "libraries/emulate_intel/include/emulate.h"

// MARK: SUB 1 - I_SUB
emu_result_t decode_sub(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_sub(emulator_8086_t* emulator, uint8_t byte1);

// MARK: SUB 2 - I_SUB_IMMEDIATE
emu_result_t decode_sub_immediate(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_sub_immediate(emulator_8086_t* emulator, uint8_t byte1);

// MARK: SUB 3 - I_SUB_IMMEDIATE_TO_AX


#endif // DECODE_SUB_H