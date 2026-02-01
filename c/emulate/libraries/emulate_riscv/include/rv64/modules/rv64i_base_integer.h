/**
 * Handles all "I" base integer instructions
 *
 * Resources:
 * * Core "I" definition: https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#rv32
 * * Table containing all instructions and their formatting:
 * https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#rv32-64g
 */

#ifndef EMU_RV64I_BASE_INTEGER_H
#define EMU_RV64I_BASE_INTEGER_H

#include <stdint.h>

#include "rv64/rv64_hart.h"
#include "rv64/rv64_instructions.h"
#include "shared/include/result.h"

emu_result_t rv64i_base_integer_emulate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
);

emu_result_t rv64i_zicsr_emulate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
);

#endif  // EMU_RV64I_BASE_INTEGER_H
