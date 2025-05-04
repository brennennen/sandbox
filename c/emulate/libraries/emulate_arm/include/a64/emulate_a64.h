/**
 * Emulator for 8086 assembly per the "8086 family users manual 1".
 */
#ifndef EMULATE_A64_H
#define EMULATE_A64_H

#include <stdio.h>
#include <stdint.h>

#include "shared/include/result.h"

#include "a64/registers_a64.h"

// TODO: probably best to make this on the heap
#define MEMORY_SIZE 65535 // 64KB,
#define PROGRAM_START 0x100 // address '0' is usually a forced segfault, write program to some
                            // offset above and leave bytes around 0 for error detection.

typedef struct {
    registers_a64_t registers;
    int instructions_count;
    // uint16_t stack_size; // using a size here in case i want to make this dynamic/resizable later.
    // uint16_t stack_top;
    // uint32_t stack[STACK_SIZE];
    uint16_t memory_size;
    uint8_t memory[MEMORY_SIZE];
} emulator_a64_t;

emu_result_t emu_a64_init(emulator_a64_t* emulator);

result_t emu_a64_decode_file(emulator_a64_t* emulator, char* input_path, char* out_buffer,
    size_t out_buffer_size);
result_t emu_a64_decode_chunk(emulator_a64_t* emulator, char* in_buffer, size_t in_buffer_size,
    char* out_buffer, size_t out_buffer_size);
result_t emu_a64_decode(emulator_a64_t* emulator, char* out_buffer, size_t out_buffer_size);



#endif // EMULATE_A64_H

