

#ifndef DECODE_ADD_H
#define DECODE_ADD_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/decode8086.h"

// MARK: ADD 1 - I_ADD
emu_result_t decode_add(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_add(emulator_t* emulator, uint8_t byte1);

// MARK: ADD 2 - I_ADD_IMMEDIATE
emu_result_t decode_add_immediate(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_add_immediate(emulator_t* emulator, uint8_t byte1);

// MARK: ADD 3 - I_ADD_IMMEDIATE_TO_AX


#endif