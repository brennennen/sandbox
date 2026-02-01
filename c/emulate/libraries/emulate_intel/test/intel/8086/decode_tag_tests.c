#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "8086/decode_8086_tag.h"
#include "8086/instruction_tags_8086.h"

// MARK: MOV
Test(emu_8086_decode_tag_tests, MOVE) {
    cr_assert(I_MOVE == emu_8086_decode_tag(0b10001000, 0));
    cr_assert(I_MOVE_IMMEDIATE == emu_8086_decode_tag(0b11000110, 0));
    cr_assert(I_MOVE_IMMEDIATE_TO_REGISTER == emu_8086_decode_tag(0b10110000, 0));
    cr_assert(I_MOVE_TO_AX == emu_8086_decode_tag(0b10100000, 0));
    cr_assert(I_MOVE_AX == emu_8086_decode_tag(0b10100010, 0));
    cr_assert(I_MOVE_TO_SEGMENT_REGISTER == emu_8086_decode_tag(0b10001110, 0));
    cr_assert(I_MOVE_SEGMENT_REGISTER == emu_8086_decode_tag(0b10001100, 0));
}

// MARK: PUSH
Test(emu_8086_decode_tag_tests, PUSH) {
    cr_assert(I_PUSH == emu_8086_decode_tag(0b11111111, 0b00110000));
    cr_assert(I_PUSH_REGISTER == emu_8086_decode_tag(0b01010001, 0));
    // cr_assert(I_PUSH_SEGMENT_REGISTER == emu_8086_decode_tag(0b00000110, 0));
}

// MARK: POP
Test(emu_8086_decode_tag_tests, POP) {
    cr_assert(I_POP == emu_8086_decode_tag(0b10001111, 0b00000000));
    cr_assert(I_POP_REGISTER == emu_8086_decode_tag(0b01011001, 0));
    // cr_assert(I_POP_SEGMENT_REGISTER == emu_8086_decode_tag(0b00000111, 0));
}

// MARK: XCHG
Test(emu_8086_decode_tag_tests, XCHG) {
    cr_assert(I_EXCHANGE == emu_8086_decode_tag(0b10000110, 0));
    cr_assert(I_EXCHANGE_AX == emu_8086_decode_tag(0b10010000, 0));
}

// MARK: IN
Test(emu_8086_decode_tag_tests, IN) {
    cr_assert(I_INPUT_FROM_FIXED_PORT == emu_8086_decode_tag(0b11100100, 0));
    cr_assert(I_INPUT_FROM_VARIABLE_PORT == emu_8086_decode_tag(0b11101100, 0));
}

// MARK: OUT
Test(emu_8086_decode_tag_tests, OUT) {
    cr_assert(I_OUTPUT_TO_FIXED_PORT == emu_8086_decode_tag(0b11100110, 0));
    cr_assert(I_OUTPUT_TO_VARIABLE_PORT == emu_8086_decode_tag(0b11101110, 0));
}

// MARK: ARITHMETIC

// MARK: ADD
Test(emu_8086_decode_tag_tests, ADD) {
    cr_assert(I_ADD == emu_8086_decode_tag(0b00000000, 0));
    cr_assert(I_ADD_IMMEDIATE == emu_8086_decode_tag(0b10000000, 0b00000000));
    cr_assert(I_ADD_IMMEDIATE_TO_AX == emu_8086_decode_tag(0b00000100, 0));
}

// MARK: ADC
Test(emu_8086_decode_tag_tests, ADC) {
    cr_assert(I_ADC == emu_8086_decode_tag(0b00010000, 0));
    cr_assert(I_ADC_IMMEDIATE == emu_8086_decode_tag(0b10000000, 0b00010000));
    cr_assert(I_ADC_IMMEDIATE_TO_AX == emu_8086_decode_tag(0b00010100, 0));
}

// MARK: INC
Test(emu_8086_decode_tag_tests, INC) {
    cr_assert(I_INC == emu_8086_decode_tag(0b11111110, 0));
    cr_assert(I_INC_REGISTER == emu_8086_decode_tag(0b01000000, 0));
}

// MARK: AAA
Test(emu_8086_decode_tag_tests, AAA) {
    cr_assert(I_AAA_ASCII_ADJUST_FOR_ADD == emu_8086_decode_tag(0b00110111, 0));
}

// MARK: DAA
Test(emu_8086_decode_tag_tests, DAA) {
    cr_assert(I_DAA_DECIMAL_ADJUST_FOR_ADD == emu_8086_decode_tag(0b00100111, 0));
}

// MARK: SUB
Test(emu_8086_decode_tag_tests, SUB) {
    cr_assert(I_SUB == emu_8086_decode_tag(0b00101000, 0));
    cr_assert(I_SUB_IMMEDIATE == emu_8086_decode_tag(0b10000000, 0b00101000));
    cr_assert(I_SUB_IMMEDIATE_FROM_AX == emu_8086_decode_tag(0b00101100, 0));
}

// MARK: SBB
Test(emu_8086_decode_tag_tests, SBB) {
    cr_assert(I_SBB == emu_8086_decode_tag(0b00011000, 0));
    cr_assert(I_SBB_IMMEDIATE == emu_8086_decode_tag(0b10000000, 0b00011000));
    cr_assert(I_SBB_IMMEDIATE_FROM_AX == emu_8086_decode_tag(0b00011100, 0));
}

// MARK: DEC
Test(emu_8086_decode_tag_tests, DEC) {
    cr_assert(I_DEC_REGISTER_OR_MEMORY == emu_8086_decode_tag(0b11111110, 0b00001000));
    cr_assert(I_DEC_REGISTER == emu_8086_decode_tag(0b01001000, 0));
}

// MARK: NEG
Test(emu_8086_decode_tag_tests, NEG) {
    cr_assert(I_NEGATE_CHANGE_SIGN == emu_8086_decode_tag(0b11110110, 0b00011000));
}

// MARK: CMP
Test(emu_8086_decode_tag_tests, CMP) {
    cr_assert(I_COMPARE == emu_8086_decode_tag(0b00111000, 0));
    cr_assert(I_COMPARE_IMMEDIATE == emu_8086_decode_tag(0b10000000, 0b00111000));
    cr_assert(I_COMPARE_IMMEDIATE_WITH_AX == emu_8086_decode_tag(0b00111100, 0));
}

// MARK: LOGIC
// MARK: NOT
Test(emu_8086_decode_tag_tests, NOT) {
    cr_assert(I_NOT == emu_8086_decode_tag(0b11110110, 0b00010000));
}

// MARK: SHL
Test(emu_8086_decode_tag_tests, SHL) {
    cr_assert(I_SHIFT_LOGICAL_LEFT == emu_8086_decode_tag(0b11010000, 0b00100000));
}

// MARK: SHR
Test(emu_8086_decode_tag_tests, SHR) {
    cr_assert(I_SHIFT_LOGICAL_RIGHT == emu_8086_decode_tag(0b11010000, 0b00101000));
}

// MARK: SAR
Test(emu_8086_decode_tag_tests, SAR) {
    cr_assert(I_SHIFT_ARITHMETIC_RIGHT == emu_8086_decode_tag(0b11010000, 0b00111000));
}

// MARK: ROL
Test(emu_8086_decode_tag_tests, ROL) {
    cr_assert(I_ROTATE_LEFT == emu_8086_decode_tag(0b11010000, 0b00000000));
}

// MARK: ROR
Test(emu_8086_decode_tag_tests, ROR) {
    cr_assert(I_ROTATE_RIGHT == emu_8086_decode_tag(0b11010000, 0b00001000));
}

// MARK: RCL
Test(emu_8086_decode_tag_tests, RCL) {
    cr_assert(I_ROTATE_LEFT_CARRY == emu_8086_decode_tag(0b11010000, 0b00010000));
}

// MARK: RCR
Test(emu_8086_decode_tag_tests, RCR) {
    cr_assert(I_ROTATE_RIGHT_CARRY == emu_8086_decode_tag(0b11010000, 0b00011000));
}

// MARK: AND
Test(emu_8086_decode_tag_tests, AND) {
    cr_assert(I_AND == emu_8086_decode_tag(0b00100000, 0));
    cr_assert(I_AND_IMMEDIATE == emu_8086_decode_tag(0b10000000, 0b00100000));
    cr_assert(I_AND_IMMEDIATE_TO_AX == emu_8086_decode_tag(0b00100100, 0));
}

// MARK: TEST
Test(emu_8086_decode_tag_tests, TEST) {
    // TODO: TEST datasheet opcodes overlap with ADC opcodes, probably a typo.
    // cr_assert(I_TEST == emu_8086_decode_tag(0b00100000, 0));
    cr_assert(I_TEST_IMMEDIATE == emu_8086_decode_tag(0b11110110, 0b00000000));
    cr_assert(I_TEST_IMMEDIATE_TO_AX == emu_8086_decode_tag(0b10101000, 0));
}

// MARK: OR
Test(emu_8086_decode_tag_tests, OR) {
    cr_assert(I_OR == emu_8086_decode_tag(0b00001000, 0));
    cr_assert(I_OR_IMMEDIATE == emu_8086_decode_tag(0b10000000, 0b00001000));
    cr_assert(I_OR_IMMEDIATE_TO_AX == emu_8086_decode_tag(0b00001100, 0));
}

// MARK: XOR
Test(emu_8086_decode_tag_tests, XOR) {
    cr_assert(I_XOR == emu_8086_decode_tag(0b00110000, 0));
    cr_assert(I_XOR_IMMEDIATE == emu_8086_decode_tag(0b10000000, 0b00110000));
    cr_assert(I_XOR_IMMEDIATE_TO_AX == emu_8086_decode_tag(0b00110100, 0));
}

// STRING MANIPULATION

// CONTROL TRANSFER

// MARK: PROCESSOR CONTROL
// MARK: CLC
Test(emu_8086_decode_tag_tests, CLC) {
    cr_assert(I_CLEAR_CARRY == emu_8086_decode_tag(0b11111000, 0));
}

// MARK: CMC
Test(emu_8086_decode_tag_tests, CMC) {
    cr_assert(I_COMPLEMENT_CARRY == emu_8086_decode_tag(0b11110101, 0));
}

// MARK: STC
Test(emu_8086_decode_tag_tests, STC) {
    cr_assert(I_SET_CARRY == emu_8086_decode_tag(0b11111001, 0));
}

// MARK: CLD
Test(emu_8086_decode_tag_tests, CLD) {
    cr_assert(I_CLEAR_DIRECTION == emu_8086_decode_tag(0b11111100, 0));
}

// MARK: STD
Test(emu_8086_decode_tag_tests, STD) {
    cr_assert(I_SET_DIRECTION == emu_8086_decode_tag(0b11111101, 0));
}

// MARK: CLI
Test(emu_8086_decode_tag_tests, CLI) {
    cr_assert(I_CLEAR_INTERRUPT == emu_8086_decode_tag(0b11111010, 0));
}

// MARK: STI
Test(emu_8086_decode_tag_tests, STI) {
    cr_assert(I_SET_INTERRUPT == emu_8086_decode_tag(0b11111011, 0));
}

// MARK: HLT
Test(emu_8086_decode_tag_tests, HLT) {
    cr_assert(I_HALT == emu_8086_decode_tag(0b11110100, 0));
}

// MARK: WAIT
Test(emu_8086_decode_tag_tests, WAIT) {
    cr_assert(I_WAIT == emu_8086_decode_tag(0b10011011, 0));
}

// MARK: ESC
Test(emu_8086_decode_tag_tests, ESC) {
    cr_assert(I_ESCAPE == emu_8086_decode_tag(0b11011000, 0));
}

// MARK: LOCK
Test(emu_8086_decode_tag_tests, LOCK) {
    cr_assert(I_LOCK == emu_8086_decode_tag(0b11110000, 0));
}

// MARK: SEGMENT
Test(emu_8086_decode_tag_tests, SEGMENT) {
    cr_assert(I_SEGMENT == emu_8086_decode_tag(0b00100110, 0));
}
