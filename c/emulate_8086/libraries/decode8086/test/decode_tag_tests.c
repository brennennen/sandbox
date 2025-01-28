#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/decode8086/include/decode8086.h"
#include "libraries/decode8086/include/decode_tag.h"

// MARK: MOV
Test(dcd_decode_tag_tests, I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY) {
    instruction_tag_t output_tag = dcd_decode_tag(0b10001000, 0);
    cr_assert(I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY == output_tag,
        "Expected I_MOVE_REGISTER_OR_MEMORY_TO_OR_FROM_REGISTER_OR_MEMORY opcode, got: %d", output_tag);
}

Test(dcd_decode_tag_tests, I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY) {
    instruction_tag_t output_tag = dcd_decode_tag(0b11000110, 0);
    cr_assert(I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY == output_tag,
        "Expected I_MOVE_IMMEDIATE_TO_REGISTER_OR_MEMORY opcode, got: %d", output_tag);
}

Test(dcd_decode_tag_tests, I_MOVE_IMMEDIATE_TO_REGISTER) {
    instruction_tag_t output_tag = dcd_decode_tag(0b10110000, 0);
    cr_assert(I_MOVE_IMMEDIATE_TO_REGISTER == output_tag,
        "Expected I_MOVE_IMMEDIATE_TO_REGISTER opcode, got: %d", output_tag);
}

Test(dcd_decode_tag_tests, I_MOVE_MEMORY_TO_ACCUMULATOR) {
    instruction_tag_t output_tag = dcd_decode_tag(0b10100000, 0);
    cr_assert(I_MOVE_MEMORY_TO_ACCUMULATOR == output_tag,
        "Expected I_MOVE_MEMORY_TO_ACCUMULATOR opcode, got: %d", output_tag);
}

Test(dcd_decode_tag_tests, I_MOVE_ACCUMULATOR_TO_MEMORY) {
    instruction_tag_t output_tag = dcd_decode_tag(0b10100010, 0);
    cr_assert(I_MOVE_ACCUMULATOR_TO_MEMORY == output_tag,
        "Expected I_MOVE_ACCUMULATOR_TO_MEMORY opcode, got: %d", output_tag);
}

Test(dcd_decode_tag_tests, I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER) {
    instruction_tag_t output_tag = dcd_decode_tag(0b10001110, 0);
    cr_assert(I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER == output_tag,
        "Expected I_MOVE_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER opcode, got: %d", output_tag);
}

Test(dcd_decode_tag_tests, I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY) {
    instruction_tag_t output_tag = dcd_decode_tag(0b10001100, 0);
    cr_assert(I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY == output_tag,
        "Expected I_MOVE_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY opcode, got: %d", output_tag);
}

// MARK: PUSH
// MARK: POP
// MARK: XCHG
// MARK: IN
// MARK: OUT
// MARK: ARITHMETIC

// MARK: ADD
Test(dcd_decode_tag_tests, I_ADD) {
    instruction_tag_t output_tag = dcd_decode_tag(0b00000000, 0);
    cr_assert(I_ADD == output_tag,
        "Expected I_ADD opcode, got: %d", output_tag);
}

Test(dcd_decode_tag_tests, I_ADD_IMMEDIATE) {
    instruction_tag_t output_tag = dcd_decode_tag(0b10000000, 0b00000000);
    cr_assert(I_ADD_IMMEDIATE == output_tag,
        "Expected I_ADD_IMMEDIATE opcode, got: %d", output_tag);
}

Test(dcd_decode_tag_tests, I_ADD_IMMEDIATE_TO_AX) {
    instruction_tag_t output_tag = dcd_decode_tag(0b00000100, 0);
    cr_assert(I_ADD_IMMEDIATE_TO_AX == output_tag,
        "Expected I_ADD_IMMEDIATE_TO_AX opcode, got: %d", output_tag);
}

// MARK: ADC
Test(dcd_decode_tag_tests, I_ADC_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER) {
    instruction_tag_t output_tag = dcd_decode_tag(0b00010000, 0);
    cr_assert(I_ADC_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER == output_tag,
        "Expected I_ADC_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER opcode, got: %d", output_tag);
}

Test(dcd_decode_tag_tests, I_ADC_IMMEDIATE_TO_REGISTER_OR_MEMORY) {
    instruction_tag_t output_tag = dcd_decode_tag(0b10000000, 0b00010000);
    cr_assert(I_ADC_IMMEDIATE_TO_REGISTER_OR_MEMORY == output_tag,
        "Expected I_ADC_IMMEDIATE_TO_REGISTER_OR_MEMORY opcode, got: %d", output_tag);
}

Test(dcd_decode_tag_tests, I_ADC_IMMEDIATE_TO_ACCUMULATOR) {
    instruction_tag_t output_tag = dcd_decode_tag(0b00010100, 0);
    cr_assert(I_ADC_IMMEDIATE_TO_ACCUMULATOR == output_tag,
        "Expected I_ADC_IMMEDIATE_TO_ACCUMULATOR opcode, got: %d", output_tag);
}

// MARK: INC
Test(dcd_decode_tag_tests, I_INC_REGISTER_OR_MEMORY) {
    instruction_tag_t output_tag = dcd_decode_tag(0b11111110, 0);
    cr_assert(I_INC_REGISTER_OR_MEMORY == output_tag,
        "Expected I_INC_REGISTER_OR_MEMORY opcode, got: %d", output_tag);
}

Test(dcd_decode_tag_tests, I_INC_REGISTER) {
    instruction_tag_t output_tag = dcd_decode_tag(0b01000000, 0);
    cr_assert(I_INC_REGISTER == output_tag,
        "Expected I_INC_REGISTER opcode, got: %d", output_tag);
}

// Test(dcd_decode_tag_tests, I_AAA_ASCII_ADJUST_FOR_ADD) {
//     instruction_tag_t output_tag = dcd_decode_tag(0b00110111, 0);
//     cr_assert(I_AAA_ASCII_ADJUST_FOR_ADD == output_tag,
//         "Expected I_AAA_ASCII_ADJUST_FOR_ADD opcode, got: %d", output_tag);
// }

// Test(dcd_decode_tag_tests, I_DAA_DECIMAL_ADJUST_FOR_ADD) {
//     instruction_tag_t output_tag = dcd_decode_tag(0b00100111, 0);
//     cr_assert(I_DAA_DECIMAL_ADJUST_FOR_ADD == output_tag,
//         "Expected I_DAA_DECIMAL_ADJUST_FOR_ADD opcode, got: %d", output_tag);
// }

// MARK: SUB
Test(dcd_decode_tag_tests, I_SUB) {
    instruction_tag_t output_tag = dcd_decode_tag(0b00101000, 0);
    cr_assert(I_SUB == output_tag,
        "Expected I_SUB opcode, got: %d", output_tag);
}

Test(dcd_decode_tag_tests, I_SUB_IMMEDIATE_FROM_REGISTER_OR_MEMORY) {
    instruction_tag_t output_tag = dcd_decode_tag(0b10000000, 0b00101000);
    cr_assert(I_SUB_IMMEDIATE_FROM_REGISTER_OR_MEMORY == output_tag,
        "Expected I_SUB_IMMEDIATE_FROM_REGISTER_OR_MEMORY opcode, got: %d", output_tag);
}

Test(dcd_decode_tag_tests, I_SUB_IMMEDIATE_FROM_ACCUMULATOR) {
    instruction_tag_t output_tag = dcd_decode_tag(0b00101100, 0);
    cr_assert(I_SUB_IMMEDIATE_FROM_ACCUMULATOR == output_tag,
        "Expected I_SUB_IMMEDIATE_FROM_ACCUMULATOR opcode, got: %d", output_tag);
}

