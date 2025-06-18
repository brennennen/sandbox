/**
 * Emulator for RV64I
 */
#ifndef EMULATE_RV64_H
#define EMULATE_RV64_H

#include <stdio.h>
#include <stdint.h>

#include "shared/include/result.h"

#include "rv64/rv64_registers.h"

// TODO: probably best to make this on the heap
#define MEMORY_SIZE 65535 // 64KB,
#define PROGRAM_START 0x100 // address '0' is usually a forced segfault, write program to some
                            // offset above and leave bytes around 0 for error detection.

typedef struct {
    registers_rv64_t registers;
    int instructions_count;
    // uint16_t stack_size; // using a size here in case i want to make this dynamic/resizable later.
    // uint16_t stack_top;
    // uint32_t stack[STACK_SIZE];
    uint16_t memory_size;
    uint8_t memory[MEMORY_SIZE];
} emulator_rv64_t;

emu_result_t emu_rv64_init(emulator_rv64_t* emulator);

char* emu_rv64_map_register_name(uint8_t reg_id);

void debug_print_registers(emulator_rv64_t* emulator);

result_t emu_rv64_disassemble_file(emulator_rv64_t* emulator, char* input_path, char* out_buffer,
    size_t out_buffer_size);
result_t emu_rv64_disassemble_chunk(emulator_rv64_t* emulator, char* in_buffer, size_t in_buffer_size,
    char* out_buffer, size_t out_buffer_size);
result_t emu_rv64_disassemble(emulator_rv64_t* emulator, char* out_buffer, size_t out_buffer_size);

result_t emu_rv64_emulate_file(emulator_rv64_t* emulator, char* input_path);
result_t emu_rv64_emulate_chunk(emulator_rv64_t* emulator, char* in_buffer, size_t in_buffer_size);
result_t emu_rv64_emulate(emulator_rv64_t* emulator);

void emu_rv64_print_registers(emulator_rv64_t* emulator);
void emu_rv64_print_registers_condensed(emulator_rv64_t* emulator);

#endif // EMULATE_RV64I_H
