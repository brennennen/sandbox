

#ifndef RV64_DISASSEMBLE
#define RV64_DISASSEMBLE

#include <stdio.h>
#include <stdint.h>

#include "rv64/rv64_emulate.h"
#include "rv64/rv64_instructions.h"


typedef struct rv64_disassembler_s {
    uint64_t registers[32];
    uint32_t pc;
    vector_register_t vector_registers[32];
    rv64_csrs_t csrs;
    int instructions_count;
    uint16_t memory_size;
    uint8_t memory[MEMORY_SIZE];
    rv64_shared_system_t shared_system;
} rv64_disassembler_t;

emu_result_t rv64_disassemble_init(rv64_disassembler_t* disassembler);

emu_result_t rv64_disassemble_r_register_register(
    rv64_disassembler_t* disassembler,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
);

emu_result_t rv64_disassemble_i_register_immediate(
    rv64_disassembler_t* disassembler,
    uint32_t raw_instruction,
    instruction_tag_rv64_t tag,
    char* buffer,
    int* index,
    size_t buffer_size
);

result_t rv64_disassemble_file(rv64_disassembler_t* disassembler, char* input_path, char* out_buffer,
    size_t out_buffer_size);
result_t rv64_disassemble_chunk(rv64_disassembler_t* disassembler, char* in_buffer, size_t in_buffer_size,
    char* out_buffer, size_t out_buffer_size);
result_t rv64_disassemble(rv64_disassembler_t* disassembler, char* out_buffer, size_t out_buffer_size);

#endif // RV64_DISASSEMBLE
