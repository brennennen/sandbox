/**
 * `NOT destination`
 * NOT inverts the bits (forms the one's complement) of the boyte or word operand (8086 Family
 * Users Manual, page 2-47, pdf page ~53).
 *
 * NOT (invert) has 1 machine instruction on 8086. See datasheet table 4-12 (8086 Family
 * Users Manual, page 4-27, pdf page ~166).
 */

#ifndef EMU_NOT_H
#define EMU_NOT_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"

// MARK: NOT
emu_result_t decode_not(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size
);
emu_result_t emu_not(emulator_t* emulator, uint8_t byte1);

#endif // EMU_NOT_H
