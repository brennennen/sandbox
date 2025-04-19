/**
 * Emulator for 8086 assembly per the "8086 family users manual 1".
 */
#ifndef DECODE_8086_H
#define DECODE_8086_H

#include <stdio.h>

#include "shared/include/result.h"
#include "shared/include/instructions.h"
#include "shared/include/registers.h"

#define STACK_SIZE 4096
#define MEMORY_SIZE 65535 // 64KB (need to add segment register support to address more space)
#define PROGRAM_START 0x100 // address '0' is usually a forced segfault, write program to some
                            // offset above and leave bytes around 0 for error detection.

#define DEBUG 1

typedef enum {
    BITS_16, // 8086 (partially implemented)
    BITS_32, // i386, x86 (not implemented)
    BITS_64, // x64 (not implemented)
} bits_mode_t;

typedef struct {
    bits_mode_t bits_mode;
    registers_t registers;
    int instructions_count;
    uint16_t stack_size; // using a size here in case i want to make this dynamic/resizable later.
    uint16_t stack_top;
    uint16_t stack[STACK_SIZE];
    uint16_t memory_size;
    uint8_t memory[MEMORY_SIZE];
    //instruction_t* instructions;
    //int instructions_capacity;
    // callbacks?
} emulator_t;

typedef enum {
    ER_SUCCESS,
    ER_FAILURE,
    ER_OUT_OF_BOUNDS,
    ER_UNKNOWN_OPCODE,
    ER_UNIMPLEMENTED_INSTRUCTION,
} emu_result_t;

static char emulate_result_strings[][32] = {
    "Success",
    "Failure",
    "Out-Of-Bounds",
    "Unknown Opcode",
    "Unimplemented Instruction",
};


void emu_init(emulator_t* emulator);

result_t emu_memory_set_byte(emulator_t* emulator, uint32_t address, uint8_t value);
result_t emu_memory_set_uint16(emulator_t* emulator, uint32_t address, uint16_t value);

result_t emu_memory_get_byte(emulator_t* emulator, uint32_t address, uint8_t* out_value);
result_t emu_memory_get_uint16(emulator_t* emulator, uint32_t address, uint16_t* out_value);

result_t emu_decode_file(emulator_t* emulator, char* input_path, char* out_buffer,
    size_t out_buffer_size);
result_t emu_decode_chunk(emulator_t* emulator, char* in_buffer, size_t in_buffer_size,
    char* out_buffer, size_t out_buffer_size);
result_t emu_decode(emulator_t* decoder, char* out_buffer, size_t out_buffer_size);

result_t emu_emulate_file(emulator_t* emulator, char* input_path);
result_t emu_emulate_chunk(emulator_t* emulator, char* in_buffer, size_t in_buffer_size);
result_t emu_emulate(emulator_t* decoder);

#endif
