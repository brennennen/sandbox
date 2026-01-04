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

typedef enum {
    RV64F_CLASS_NEGATIVE_INFINITY = 0,
    RV64F_CLASS_NEGATIVE_NORMAL = 1,
    RV64F_CLASS_NEGATIVE_SUBNORMAL = 2,
    RV64F_CLASS_NEGATIVE_ZERO = 3,
    RV64F_CLASS_POSITIVE_ZERO = 4,
    RV64F_CLASS_POSITIVE_SUBNORMAL = 5,
    RV64F_CLASS_POSITIVE_NORMAL = 6,
    RV64F_CLASS_POSITIVE_INFINITY = 7,
    RV64F_CLASS_SIGNALING_NAN = 8,
    RV64F_CLASS_QUIET_NAN = 9
} rv64_class_t;

emu_result_t rv64f_float_emulate(
    rv64_hart_t* hart,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
);

#endif // EMU_RV64F_FLOAT_H
