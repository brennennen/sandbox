/**
 * `CMC`
 * CMC (Complement Carry Flag) "toggles" CF to its opposite state and affects
 * no other flags(8086 Family Users Manual, page 2-47, pdf page ~62).
 *
 * CMC has 1 machine instruction on 8086. See datasheet table 4-12 (8086 Family Users Manual,
 * page 4-27, pdf page ~169).
 *
 */

#ifndef DECODE_CMC_H
#define DECODE_CMC_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"

// MARK: CMC
emu_result_t decode_cmc(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_cmc(emulator_t* emulator, uint8_t byte1);

#endif // DECODE_CMC_H
