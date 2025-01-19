/**
 * 
 * 
 * 
 * 
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
} decoder_t;


void dcd_init(decoder_t* decoder, instruction_t* instructions, size_t instructions_size);

result_t decode_file(decoder_t* decoder, char* input_path);
result_t decode_chunk(decoder_t* decoder, char* buffer, size_t buffer_size);
result_t decode(decoder_t* decoder);

result_t read_opcode(uint8_t byte, instruction_tag_t* out_instruction_tag);

// MARK: MOV
/**
 * 
 */
result_t decode__move_register_or_memory_to_or_from_register_or_memory(
    decoder_t* decoder,  
    move_register_or_memory_to_or_from_register_or_memory_t *out_move
);
result_t decode__move_immediate_to_register_or_memory(
    decoder_t* decoder, 
    move_immediate_to_register_or_memory_t* out_move
);
result_t decode__move_immediate_to_register(
    decoder_t* decoder, 
    move_immediate_to_register_t* out_move
);



// MARK: PUSH
// MARK: POP
// MARK: XCHG
// MARK: ...



void write__instruction(instruction_t* instruction, char* buffer, int buffer_size);

#endif
