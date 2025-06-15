/**
 * Emulator for 8086 assembly per the "8086 family users manual 1".
 */
#ifndef EMULATE_8086_H
#define EMULATE_8086_H

#include <stdio.h>
#include <stdint.h>

#include "shared/include/result.h"

#include "8086/registers_8086.h"


#define STACK_SIZE 4096
#define MEMORY_SIZE 65535 // 64KB (need to add segment register support to address more space)
#define PROGRAM_START 0x100 // address '0' is usually a forced segfault, write program to some
                            // offset above and leave bytes around 0 for error detection.

typedef struct {
    registers_8086_t registers;
    int instructions_count;
    uint16_t stack_size; // using a size here in case i want to make this dynamic/resizable later.
    uint16_t stack_top;
    uint16_t stack[STACK_SIZE];
    uint16_t memory_size;
    uint8_t memory[MEMORY_SIZE];
} emulator_8086_t;

emu_result_t emu_8086_init(emulator_8086_t* emulator);

result_t emu_8086_disassemble_file(emulator_8086_t* emulator, char* input_path, char* out_buffer,
    size_t out_buffer_size);
result_t emu_8086_disassemble_chunk(emulator_8086_t* emulator, char* in_buffer, size_t in_buffer_size,
    char* out_buffer, size_t out_buffer_size);
result_t emu_8086_disassemble(emulator_8086_t* emulator, char* out_buffer, size_t out_buffer_size);

result_t emu_8086_emulate(emulator_8086_t* emulator);
result_t emu_8086_emulate_file(emulator_8086_t* emulator, char* input_path);
result_t emu_8086_emulate_chunk(emulator_8086_t* emulator, char* in_buffer, size_t in_buffer_size);

emu_result_t emu_memory_set_byte(emulator_8086_t* emulator, uint32_t address, uint8_t value);
emu_result_t emu_memory_set_uint16(emulator_8086_t* emulator, uint32_t address, uint16_t value);

emu_result_t emu_memory_get_byte(emulator_8086_t* emulator, uint32_t address, uint8_t* out_value);
emu_result_t emu_memory_get_uint16(emulator_8086_t* emulator, uint32_t address, uint16_t* out_value);

#endif // EMULATE_8086_H
