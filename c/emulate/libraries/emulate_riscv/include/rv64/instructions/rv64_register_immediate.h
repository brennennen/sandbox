/**
 * Handles all "I" (register-immediate) core instructions.
 *
 * Resources:
 * * Core instruction format "I" (register-register) definition: https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_integer_register_immediate_instructions
 * * Table containing all instructions and their formatting: https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#rv32-64g
 */

#ifndef EMU_RV64_REGISTER_IMMEDIATE_H
#define EMU_RV64_REGISTER_IMMEDIATE_H

#include <stdio.h>
#include <stdint.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

emu_result_t rv64_disassemble_register_immediate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
);

emu_result_t rv64_emulate_register_immediate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag
);

#endif // EMU_RV64I_REGISTER_IMMEDIATE_H
