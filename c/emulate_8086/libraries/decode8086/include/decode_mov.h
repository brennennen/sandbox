/**
 * Responsible for decoding the "mov" assembly instruction for the 8086. This instruction
 * can be mapped into 7 different opcode encodings. The order of these encodings will
 * follow the datasheet table 4-12.
 * 
 * 1. I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY
 * 2. I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY
 * 3. I_MOVE_IMMEDIATE_TO_REGISTER
 * 4. I_MOVE_MEMORY_TO_ACCUMULATOR
 * 5. I_MOVE_ACCUMULATOR_TO_MEMORY
 * 6. I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER
 * 7. I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY
 * 
 * NOTE:
 * These names are unfortunately long, but that's how the 8086 designers decided to split
 * up the work. I kicked around naming them "I_MOVE_1", "I_MOVE_2", etc. but that required
 * me to constantly look up which was which. I'm not happy with either direction, but
 * sticking with the long names for now.
 */
#ifndef DECODE_MOV_H
#define DECODE_MOV_H

// MARK: 1. I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY
decode_result_t decode__move_register_or_memory_to_or_from_register_or_memory(
    decoder_t* decoder,
    uint8_t byte1,
    move_register_or_memory_to_or_from_register_or_memory_t* move
);
void write__move_register_or_memory_to_or_from_register_or_memory(
    move_register_or_memory_to_or_from_register_or_memory_t* move, 
    char* buffer,
    int* index,
    int buffer_size
);

// MARK: 2. I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY
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

// MARK: 4. I_MOVE_MEMORY_TO_ACCUMULATOR
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


#endif // DECODE_MOV_H