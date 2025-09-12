/**
 * Handles all "V" Extension instructions.
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#mstandard
 *
 */

#ifndef EMU_RV64_VECTOR_H
#define EMU_RV64_VECTOR_H

#include <stdio.h>
#include <stdint.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

emu_result_t rv64v_vector_emulate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
);

#endif // EMU_RV64_VECTOR_H
