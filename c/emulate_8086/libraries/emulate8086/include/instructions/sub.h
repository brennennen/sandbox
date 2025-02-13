

#ifndef DECODE_SUB_H
#define DECODE_SUB_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"

// MARK: SUB 1 - I_SUB
emu_result_t decode_sub(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_sub(emulator_t* emulator, uint8_t byte1);

// MARK: SUB 2 - I_SUB_IMMEDIATE
// MARK: SUB 3 - I_SUB_IMMEDIATE_TO_AX


#endif // DECODE_SUB_H