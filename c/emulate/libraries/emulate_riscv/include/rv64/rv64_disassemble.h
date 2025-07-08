

#ifndef RV64_DISASSEMBLE
#define RV64_DISASSEMBLE

#include <stdio.h>
#include <stdint.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"

emu_result_t rv64_disassemble_init(emulator_rv64_t* emulator);

emu_result_t rv64_disassemble_r_register_register(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
);

emu_result_t rv64_disassemble_i_register_immediate(
    emulator_rv64_t* emulator,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
);

result_t emu_rv64_disassemble_file(emulator_rv64_t* emulator, char* input_path, char* out_buffer,
    size_t out_buffer_size);
result_t emu_rv64_disassemble_chunk(emulator_rv64_t* emulator, char* in_buffer, size_t in_buffer_size,
    char* out_buffer, size_t out_buffer_size);
result_t emu_rv64_disassemble(emulator_rv64_t* emulator, char* out_buffer, size_t out_buffer_size);

#endif // RV64_DISASSEMBLE
