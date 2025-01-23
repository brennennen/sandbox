#include <stdio.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

// TODO: have this call test funtions in other library specific test projects?

// MARK: read_opcode tests


Test(dcd_read_opcode, mov_1) {
    uint8_t input = 0b10001000;
    instruction_tag_t output_tag = I_INVALID;
    cr_assert(SUCCESS == dcd_read_opcode(input, &output_tag));
    cr_assert(I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY == output_tag);
}

Test(dcd_read_opcode, mov_2) {
    uint8_t input = 0b11000110;
    instruction_tag_t output_tag = I_INVALID;
    cr_assert(SUCCESS == dcd_read_opcode(input, &output_tag));
    cr_assert(I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY == output_tag);
}

Test(dcd_read_opcode, mov_3) {
    uint8_t input = 0b10110000;
    instruction_tag_t output_tag = I_INVALID;
    cr_assert(SUCCESS == dcd_read_opcode(input, &output_tag));
    cr_assert(I_MOVE_IMMEDIATE_TO_REGISTER == output_tag);
}

Test(dcd_read_opcode, mov_4) {
    uint8_t input = 0b10100000;
    instruction_tag_t output_tag = I_INVALID;
    cr_assert(SUCCESS == dcd_read_opcode(input, &output_tag));
    cr_assert(I_MOVE_MEMORY_TO_ACCUMULATOR == output_tag);
}

// MARK: 

// Test(dcd_read_opcode, base) {
//     uint8_t input[2] = { 0x89, 0xD9 }; //mov cx, bx
//     uint8_t input = 0b10001000;
//     instruction_tag_t output_tag = I_INVALID;
//     cr_assert(SUCCESS == dcd_read_opcode(input, &output_tag));
//     cr_assert(I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY == output_tag);
// }
