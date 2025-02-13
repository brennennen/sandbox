/**
 * Decoder for 8086 assembly per the "8086 family users manual 1".
 */
#ifndef DECODE_8086_H
#define DECODE_8086_H

#include <stdio.h>

#include "shared/include/result.h"
#include "shared/include/instructions.h"
#include "shared/include/registers.h"

typedef struct decoder {
    uint8_t* buffer;
    size_t buffer_size;
    int buffer_index;
    int current_byte;
    registers_t registers;
    //instruction_t* instructions;
    int instructions_count;
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

result_t emu_decode_file(emulator_t* emulator, char* input_path, char* out_buffer,
    size_t out_buffer_size);
result_t emu_decode_chunk(emulator_t* emulator, char* in_buffer, size_t in_buffer_size,
    char* out_buffer, size_t out_buffer_size);
result_t emu_decode(emulator_t* decoder, char* out_buffer, size_t out_buffer_size);

#endif
