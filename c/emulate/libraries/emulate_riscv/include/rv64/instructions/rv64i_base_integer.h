/**
 * Handles all "R" (register-register) core instructions.
 *
 * Resources:
 * * Core instruction format "R" (register-register) definition: https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_integer_register_register_operations
 * * Table containing all instructions and their formatting: https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#rv32-64g
 */

#ifndef EMU_RV64I_BASE_INTEGER_H
#define EMU_RV64I_BASE_INTEGER_H

#include <stdio.h>
#include <stdint.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

emu_result_t rv64i_base_integer_emulate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
);

#endif // EMU_RV64I_BASE_INTEGER_H
