#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"

// MARK: MOV
Test(dcd_read_opcode_tests, I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY) {
    uint8_t input = 0b10001000;
    instruction_tag_t output_tag = I_INVALID;
    cr_assert(SUCCESS == dcd_read_opcode(input, &output_tag));
    cr_assert(I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY == output_tag, 
        "Expected I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY opcode, got: %d", output_tag);
}

Test(dcd_read_opcode_tests, I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY) {
    uint8_t input = 0b11000110;
    instruction_tag_t output_tag = I_INVALID;
    cr_assert(SUCCESS == dcd_read_opcode(input, &output_tag));
    cr_assert(I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY == output_tag, 
        "Expected I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY opcode, got: %d", output_tag);
}

Test(dcd_read_opcode_tests, I_MOVE_IMMEDIATE_TO_REGISTER) {
    uint8_t input = 0b10110000;
    instruction_tag_t output_tag = I_INVALID;
    cr_assert(SUCCESS == dcd_read_opcode(input, &output_tag));
    cr_assert(I_MOVE_IMMEDIATE_TO_REGISTER == output_tag, 
        "Expected I_MOVE_IMMEDIATE_TO_REGISTER opcode, got: %d", output_tag);
}

Test(dcd_read_opcode_tests, I_MOVE_MEMORY_TO_ACCUMULATOR) {
    uint8_t input = 0b10100000;
    instruction_tag_t output_tag = I_INVALID;
    cr_assert(SUCCESS == dcd_read_opcode(input, &output_tag));
    cr_assert(I_MOVE_MEMORY_TO_ACCUMULATOR == output_tag, 
        "Expected I_MOVE_MEMORY_TO_ACCUMULATOR opcode, got: %d", output_tag);
}

Test(dcd_read_opcode_tests, I_MOVE_ACCUMULATOR_TO_MEMORY) {
    uint8_t input = 0b10100010;
    instruction_tag_t output_tag = I_INVALID;
    cr_assert(SUCCESS == dcd_read_opcode(input, &output_tag));
    cr_assert(I_MOVE_ACCUMULATOR_TO_MEMORY == output_tag, 
        "Expected I_MOVE_ACCUMULATOR_TO_MEMORY opcode, got: %d", output_tag);
}

Test(dcd_read_opcode_tests, I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER) {
    uint8_t input = 0b10001110;
    instruction_tag_t output_tag = I_INVALID;
    cr_assert(SUCCESS == dcd_read_opcode(input, &output_tag));
    cr_assert(I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER == output_tag, 
        "Expected I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER opcode, got: %d", output_tag);
}

Test(dcd_read_opcode_tests, I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY) {
    uint8_t input = 0b10001100;
    instruction_tag_t output_tag = I_INVALID;
    cr_assert(SUCCESS == dcd_read_opcode(input, &output_tag));
    cr_assert(I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY == output_tag, 
        "Expected I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY opcode, got: %d", output_tag);
}

// MARK: PUSH


