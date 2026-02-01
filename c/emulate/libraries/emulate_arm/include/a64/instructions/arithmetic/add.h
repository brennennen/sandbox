/**
 *
 */

#ifndef EMU_A64_ADD_H
#define EMU_A64_ADD_H

#include <stdint.h>
#include <stdio.h>

#include "a64/emulate_a64.h"

emu_result_t emu_a64_decode_add_immediate(
    emulator_a64_t* emulator,
    uint32_t raw_instruction,
    char* buffer,
    int* index,
    size_t buffer_size
);

#endif  // EMU_A64_ADD_H
