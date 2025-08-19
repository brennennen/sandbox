


#ifndef EMU_RV64A_ATOMIC_DISASSEMBLE_H
#define EMU_RV64A_ATOMIC_DISASSEMBLE_H

#include <stdio.h>
#include <stdint.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

emu_result_t rv64a_atomic_disassemble(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
);

#endif // EMU_RV64A_ATOMIC_DISASSEMBLE_H
