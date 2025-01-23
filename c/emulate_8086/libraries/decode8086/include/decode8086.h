/**
 * Decoder for 8086 assembly per the "8086 family users manual 1".
 */
#ifndef DECODE_8086_H
#define DECODE_8086_H

#include <stdio.h>

#include "shared/include/result.h"
#include "shared/include/instructions.h"

typedef struct decoder {
    uint8_t* buffer;
    size_t buffer_size;
    int buffer_index;
    int current_byte;
    instruction_t* instructions;
    int instructions_count;
    int instructions_capacity;

    // callbacks?
} decoder_t;

typedef enum {
    DR_SUCCESS,
    DR_FAILURE,
    DR_OUT_OF_BOUNDS,
    DR_UNKNOWN_OPCODE,
    DR_UNIMPLEMENTED_INSTRUCTION,
} decode_result_t;

static char decode_result_strings[][32] = {
    "Success",
    "Failure",
    "Out-Of-Bounds",
    "Unknown Opcode",
    "Unimplemented Instruction",
};


void dcd_init(decoder_t* decoder, instruction_t* instructions, size_t instructions_size);

void dcd_register_mov1_callback(decoder_t* decoder);

result_t dcd_decode_file(decoder_t* decoder, char* input_path);
result_t dcd_decode_chunk(decoder_t* decoder, char* buffer, size_t buffer_size);
result_t dcd_decode(decoder_t* decoder);

decode_result_t dcd_read_opcode(uint8_t byte, instruction_tag_t* out_instruction_tag);

void dcd_write_all_assembly(instruction_t* instructions, size_t instructions_count, char* buffer, size_t buffer_size);
void dcd_write_assembly_instruction(instruction_t* instruction, char* buffer, int* index, size_t buffer_size);
#endif
