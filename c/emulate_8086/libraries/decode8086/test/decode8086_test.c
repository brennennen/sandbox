#include <stdio.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

// MARK: read_opcode tests

Test(read_opcode_tests, mov_1) {
    uint8_t input = 0b10001000;
    instruction_tag_t output_tag = I_INVALID;
    cr_assert(SUCCESS == read_opcode(input, &output_tag));
    cr_assert(I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY == output_tag);
}

Test(decode_chunk_tests, mov_1) {
    uint8_t input[2] = { 0x89, 0xD9 }; //mov cx, bx
    decoder_t decoder = {};
    instruction_t instructions[4096] = {0};
    dcd_init(&decoder, instructions, 4096);
    cr_assert(SUCCESS == decode_chunk(&decoder, input, sizeof(input)));
    cr_assert(1 == decoder.instructions_count);
    cr_assert(I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY 
        == decoder.instructions[0].tag);
    cr_assert(WIDE_WORD == decoder.instructions[0].data
        .move_register_or_memory_to_or_from_register_or_memory.wide);
    cr_assert(DIR_TO_REGISTER == decoder.instructions[0].data
        .move_register_or_memory_to_or_from_register_or_memory.direction);
    
    //cr_assert(I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY == output_tag);
}