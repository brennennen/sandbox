/**
 *
 */

#ifndef EMU_RV64I_ADD_H
#define EMU_RV64I_ADD_H

#include <stdio.h>
#include <stdint.h>

#include "rv64i/emulate_rv64i.h"

emu_result_t emu_rv64i_decode_add_immediate(
    emulator_rv64i_t* emulator,
    uint32_t raw_instruction,
    char* buffer,
    int* index,
    size_t buffer_size
);


#endif // EMU_A64_ADD_H

