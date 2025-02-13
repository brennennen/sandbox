

#ifndef DECODE_CMP_H
#define DECODE_CMP_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

// MARK: CMP 1 - I_CMP
emu_result_t decode_compare(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_compare(emulator_t* emulator, uint8_t byte1);

// MARK: CMP 2 - I_COMPARE_IMMEDIATE
// MARK: CMP 3 - I_COMPARE_IMMEDIATE_TO_AX


#endif // DECODE_CMP_H