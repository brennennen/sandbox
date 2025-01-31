/**
 * Responsible for decoding the "mov" assembly instruction for the 8086. This instruction
 * can be mapped into 7 different opcode encodings. The order of these encodings will
 * follow the datasheet table 4-12.
 *
 * 1. I_MOVE
 * 2. I_MOVE_IMMEDIATE
 * 3. I_MOVE_IMMEDIATE_TO_REGISTER
 * 4. I_MOVE_TO_AX
 * 5. I_MOVE_AX
 * 6. I_MOVE_TO_SEGMENT_REGISTER
 * 7. I_MOVE_SEGMENT_REGISTER
 *
 * NOTE:
 * These names are unfortunately long, but that's how the 8086 designers decided to split
 * up the work. I kicked around naming them "I_MOVE_1", "I_MOVE_2", etc. but that required
 * me to constantly look up which was which. I'm not happy with either direction, but
 * sticking with the long names for now.
 */
#ifndef DECODE_MOV_H
#define DECODE_MOV_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"


// MARK: 1. I_MOVE
decode_result_t decode_move(
    decoder_t* decoder,
    uint8_t byte1,
    move_t* move
);
void write_move(
    move_t* move,
    char* buffer,
    int* index,
    int buffer_size
);

// MARK: 2. I_MOVE_IMMEDIATE
decode_result_t decode__move_immediate_to_register_or_memory(
    decoder_t* decoder,
    uint8_t byte1,
    move_immediate_to_register_or_memory_t* move
);
void write__move_immediate_to_register_or_memory(
    move_immediate_to_register_or_memory_t* move,
    char* buffer,
    int* index,
    int buffer_size
);

// MARK: 3. I_MOVE_IMMEDIATE_TO_REGISTER
decode_result_t decode__move_immediate_to_register(
    decoder_t* decoder,
    uint8_t byte1,
    move_immediate_to_register_t* move
);
void write__move_immediate_to_register(
    move_immediate_to_register_t* move,
    char* buffer,
    int* index,
    int buffer_size
);

// MARK: 4. I_MOVE_TO_AX
decode_result_t decode__move_memory_to_accumulator(
    decoder_t* decoder,
    uint8_t byte1,
    move_memory_to_accumulator_t* move
);
void write__move_memory_to_accumulator(
    move_memory_to_accumulator_t* move,
    char* buffer,
    int* index,
    int buffer_size
);

// MARK: 5. I_MOVE_AX
decode_result_t decode__move_accumulator_to_memory(
    decoder_t* decoder,
    uint8_t byte1,
    move_accumulator_to_memory_t* move
);
void write__move_accumulator_to_memory(
    move_accumulator_to_memory_t* move,
    char* buffer,
    int* index,
    int buffer_size
);

#endif // DECODE_MOV_H