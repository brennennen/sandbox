/**
 * Handles all "F" (register-register) core instructions.
 *
 * Resources:
 * * "F" float definition: https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#single-float
 * * Table containing all instructions and their formatting: https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#rv32-64g
 */

#ifndef EMU_RV64F_FLOAT_H
#define EMU_RV64F_FLOAT_H

#include <stdio.h>
#include <stdint.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

emu_result_t rv64f_float_emulate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
);

#endif // EMU_RV64F_FLOAT_H
