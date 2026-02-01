/**
 * Handles all "M" Extension instructions.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#mstandard
 *
 * All "M" extension instructions follow the core instruction format "R" (register-register).
 * @see
 * https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_integer_register_register_operations
 *
 * Table containing all instructions and their formatting.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#rv32-64g
 */

#ifndef EMU_RV64_MULTIPLICATION_H
#define EMU_RV64_MULTIPLICATION_H

#include <stdint.h>

#include "rv64/rv64_hart.h"
#include "rv64/rv64_instructions.h"
#include "shared/include/result.h"

emu_result_t rv64_multiplication_emulate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
);

#endif  // EMU_RV64_MULTIPLICATION_H
